// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FFmpeg.h>

#include <tlrCore/StringFormat.h>

extern "C"
{
#include <libavutil/imgutils.h>

} // extern "C"

namespace tlr
{
    namespace ffmpeg
    {
        void Write::_init(
            const std::string& fileName,
            const io::Info& info)
        {
            IWrite::_init(fileName, info);
            
            if (info.video.empty())
            {
                throw std::runtime_error(string::Format("{0}: No video").arg(fileName));
            }

            _avOutputFormat = av_guess_format(NULL, fileName.c_str(), NULL);
            if (!_avOutputFormat)
            {
                throw std::runtime_error(string::Format("{0}: File not supported").arg(fileName));
            }
            int r = avformat_alloc_output_context2(&_avFormatContext, _avOutputFormat, NULL, fileName.c_str());
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            _avCodec = avcodec_find_encoder(_avOutputFormat->video_codec);
            if (!_avCodec)
            {
                throw std::runtime_error(string::Format("{0}: Cannot find encoder").arg(fileName));
            }
            _avVideoStream = avformat_new_stream(_avFormatContext, _avCodec);

            _avVideoStream->codec->codec_id = _avOutputFormat->video_codec;
            _avVideoStream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
            const auto& videoInfo = info.video[0];
            const auto& imageInfo = videoInfo.info;
            _avVideoStream->codec->width = imageInfo.size.w;
            _avVideoStream->codec->height = imageInfo.size.h;
            _avVideoStream->codec->sample_aspect_ratio = AVRational({ 1, 1 });
            _avVideoStream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
            const auto rational = toRational(videoInfo.duration.rate());
            _avVideoStream->codec->time_base = { rational.den, rational.num };
            _avVideoStream->codec->framerate = rational;
            if (_avFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            {
                _avVideoStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }

            r = avcodec_open2(_avVideoStream->codec, _avCodec, NULL);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }

            r = avcodec_parameters_from_context(_avVideoStream->codecpar, _avVideoStream->codec);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }

            _avVideoStream->time_base = { rational.den, rational.num };
            _avVideoStream->avg_frame_rate = rational;

            r = avio_open(&_avFormatContext->pb, fileName.c_str(), AVIO_FLAG_WRITE);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }

            r = avformat_write_header(_avFormatContext, NULL);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }

            _avPacket = av_packet_alloc();

            _avFrame = av_frame_alloc();
            _avFrame->format = _avVideoStream->codecpar->format;
            _avFrame->width = _avVideoStream->codecpar->width;
            _avFrame->height = _avVideoStream->codecpar->height;
            r = av_frame_get_buffer(_avFrame, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }

            _avFrame2 = av_frame_alloc();
            switch (imageInfo.pixelType)
            {
            case imaging::PixelType::L_U8: _avPixelFormat = AV_PIX_FMT_GRAY8; break;
            case imaging::PixelType::RGB_U8: _avPixelFormat = AV_PIX_FMT_RGB24; break;
            case imaging::PixelType::RGBA_U8: _avPixelFormat = AV_PIX_FMT_RGBA; break;
            }
            _swsContext = sws_getContext(
                imageInfo.size.w,
                imageInfo.size.h,
                _avPixelFormat,
                imageInfo.size.w,
                imageInfo.size.h,
                AV_PIX_FMT_YUV420P,
                swsScaleFlags,
                0,
                0,
                0);
        }

        Write::Write()
        {}

        Write::~Write()
        {
            if (_avFormatContext->pb)
            {
                _encodeVideo(nullptr);
                av_write_trailer(_avFormatContext);
            }

            if (_swsContext)
            {
                sws_freeContext(_swsContext);
            }
            if (_avFrame2)
            {
                av_frame_free(&_avFrame2);
            }
            if (_avFrame)
            {
                av_frame_free(&_avFrame);
            }
            if (_avPacket)
            {
                av_free_packet(_avPacket);
            }
            if (_avFormatContext->pb)
            {
                avio_closep(&_avFormatContext->pb);
            }
            if (_avFormatContext)
            {
                avformat_free_context(_avFormatContext);
            }
        }

        std::shared_ptr<Write> Write::create(
            const std::string& fileName,
            const io::Info& info)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(fileName, info);
            return out;
        }

        void Write::writeVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            const auto& info = image->getInfo();
            av_image_fill_arrays(
                _avFrame2->data,
                _avFrame2->linesize,
                image->getData(),
                _avPixelFormat,
                info.size.w,
                info.size.h,
                1);
            //! \bug This is wrong for flipping YUV data.
            //for (int i = 0; i < 4; i++)
            //{
            //    _avFrame2->data[i] += _avFrame2->linesize[i] * (info.size.h - 1);
            //    _avFrame2->linesize[i] = -_avFrame2->linesize[i];
            //}
            sws_scale(
                _swsContext,
                (uint8_t const* const*)_avFrame2->data,
                _avFrame2->linesize,
                0,
                _avVideoStream->codecpar->height,
                _avFrame->data,
                _avFrame->linesize);

            AVRational r = toRational(time.rate());
            _avFrame->pts = av_rescale_q(
                time.value(),
                AVRational({ r.den, r.num }),
                _avVideoStream->time_base);
            _encodeVideo(_avFrame);
        }

        void Write::_encodeVideo(AVFrame* frame)
        {
            int r = avcodec_send_frame(_avVideoStream->codec, frame);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: Cannot write frame").arg(_fileName));
            }

            while (r >= 0)
            {
                r = avcodec_receive_packet(_avVideoStream->codec, _avPacket);
                if (r == AVERROR(EAGAIN) || r == AVERROR_EOF)
                {
                    return;
                }
                else if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot write frame").arg(_fileName));
                }
                r = av_interleaved_write_frame(_avFormatContext, _avPacket);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot write frame").arg(_fileName));
                }
                av_packet_unref(_avPacket);
            }
        }
    }
}
