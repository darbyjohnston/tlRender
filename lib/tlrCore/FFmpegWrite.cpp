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
        struct Write::Private
        {
            std::string fileName;
            AVOutputFormat* avOutputFormat = nullptr;
            AVFormatContext* avFormatContext = nullptr;
            AVCodec* avCodec = nullptr;
            AVStream* avVideoStream = nullptr;
            AVPacket* avPacket = nullptr;
            AVFrame* avFrame = nullptr;
            AVPixelFormat avPixelFormatIn = AV_PIX_FMT_NONE;
            AVPixelFormat avPixelFormatOut = AV_PIX_FMT_YUV420P;
            AVFrame* avFrame2 = nullptr;
            SwsContext* swsContext = nullptr;
        };

        void Write::_init(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);
            
            TLR_PRIVATE_P();

            p.fileName = path.get();
            if (info.video.empty())
            {
                throw std::runtime_error(string::Format("{0}: No video").arg(p.fileName));
            }

            p.avOutputFormat = av_guess_format(NULL, p.fileName.c_str(), NULL);
            if (!p.avOutputFormat)
            {
                throw std::runtime_error(string::Format("{0}: File not supported").arg(p.fileName));
            }
            int r = avformat_alloc_output_context2(&p.avFormatContext, p.avOutputFormat, NULL, p.fileName.c_str());
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }
            Profile profile = Profile::H264;
            int avProfile = 0;
            int64_t avBitRate = 0;
            auto option = options.find("ffmpeg/WriteProfile");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> profile;
            }
            switch (profile)
            {
            case Profile::H264:
                p.avOutputFormat->video_codec = AV_CODEC_ID_H264;
                avProfile = FF_PROFILE_H264_HIGH;
                avBitRate = 100000000;
                break;
            case Profile::ProRes:
                p.avOutputFormat->video_codec = AV_CODEC_ID_PRORES;
                p.avPixelFormatOut = AV_PIX_FMT_YUV422P10;
                avProfile = FF_PROFILE_PRORES_STANDARD;
                break;
            case Profile::ProRes_Proxy:
                p.avOutputFormat->video_codec = AV_CODEC_ID_PRORES;
                p.avPixelFormatOut = AV_PIX_FMT_YUV422P10;
                avProfile = FF_PROFILE_PRORES_PROXY;
                break;
            case Profile::ProRes_LT:
                p.avOutputFormat->video_codec = AV_CODEC_ID_PRORES;
                p.avPixelFormatOut = AV_PIX_FMT_YUV422P10;
                avProfile = FF_PROFILE_PRORES_LT;
                break;
            case Profile::ProRes_HQ:
                p.avOutputFormat->video_codec = AV_CODEC_ID_PRORES;
                p.avPixelFormatOut = AV_PIX_FMT_YUV422P10;
                avProfile = FF_PROFILE_PRORES_HQ;
                break;
            case Profile::ProRes_4444:
                p.avOutputFormat->video_codec = AV_CODEC_ID_PRORES;
                p.avPixelFormatOut = AV_PIX_FMT_YUV444P10;
                avProfile = FF_PROFILE_PRORES_4444;
                break;
            case Profile::ProRes_XQ:
                p.avOutputFormat->video_codec = AV_CODEC_ID_PRORES;
                p.avPixelFormatOut = AV_PIX_FMT_YUV444P10;
                avProfile = FF_PROFILE_PRORES_XQ;
                break;
            default: break;
            }
            p.avCodec = avcodec_find_encoder(p.avOutputFormat->video_codec);
            if (!p.avCodec)
            {
                throw std::runtime_error(string::Format("{0}: Cannot find encoder").arg(p.fileName));
            }
            p.avVideoStream = avformat_new_stream(p.avFormatContext, p.avCodec);

            p.avVideoStream->codec->codec_id = p.avOutputFormat->video_codec;
            p.avVideoStream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
            const auto& videoInfo = info.video[0];
            p.avVideoStream->codec->width = videoInfo.size.w;
            p.avVideoStream->codec->height = videoInfo.size.h;
            p.avVideoStream->codec->sample_aspect_ratio = AVRational({ 1, 1 });
            p.avVideoStream->codec->pix_fmt = p.avPixelFormatOut;
            const auto rational = time::toRational(info.videoDuration.rate());
            p.avVideoStream->codec->time_base = { rational.second, rational.first };
            p.avVideoStream->codec->framerate = { rational.first, rational.second };
            p.avVideoStream->codec->profile = avProfile;
            p.avVideoStream->codec->bit_rate = avBitRate;
            if (p.avFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            {
                p.avVideoStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }

            r = avcodec_open2(p.avVideoStream->codec, p.avCodec, NULL);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }

            r = avcodec_parameters_from_context(p.avVideoStream->codecpar, p.avVideoStream->codec);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }

            p.avVideoStream->time_base = { rational.second, rational.first };
            p.avVideoStream->avg_frame_rate = { rational.first, rational.second };

            for (const auto& i : info.tags)
            {
                av_dict_set(&p.avFormatContext->metadata, i.first.c_str(), i.second.c_str(), 0);
            }

            r = avio_open(&p.avFormatContext->pb, p.fileName.c_str(), AVIO_FLAG_WRITE);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }

            r = avformat_write_header(p.avFormatContext, NULL);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }

            p.avPacket = av_packet_alloc();

            p.avFrame = av_frame_alloc();
            p.avFrame->format = p.avVideoStream->codecpar->format;
            p.avFrame->width = p.avVideoStream->codecpar->width;
            p.avFrame->height = p.avVideoStream->codecpar->height;
            r = av_frame_get_buffer(p.avFrame, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }

            p.avFrame2 = av_frame_alloc();
            switch (videoInfo.pixelType)
            {
            case imaging::PixelType::L_U8:     p.avPixelFormatIn = AV_PIX_FMT_GRAY8;   break;
            case imaging::PixelType::RGB_U8:   p.avPixelFormatIn = AV_PIX_FMT_RGB24;   break;
            case imaging::PixelType::RGBA_U8:  p.avPixelFormatIn = AV_PIX_FMT_RGBA;    break;
            case imaging::PixelType::YUV_420P: p.avPixelFormatIn = AV_PIX_FMT_YUV420P; break;
            default:
                throw std::runtime_error(string::Format("{0}: Incompatible pixel type").arg(p.fileName));
                break;
            }
            p.swsContext = sws_getContext(
                videoInfo.size.w,
                videoInfo.size.h,
                p.avPixelFormatIn,
                videoInfo.size.w,
                videoInfo.size.h,
                p.avPixelFormatOut,
                swsScaleFlags,
                0,
                0,
                0);
        }

        Write::Write() :
            _p(new Private)
        {}

        Write::~Write()
        {
            TLR_PRIVATE_P();

            if (p.swsContext)
            {
                _encodeVideo(nullptr);
                av_write_trailer(p.avFormatContext);
            }

            if (p.swsContext)
            {
                sws_freeContext(p.swsContext);
            }
            if (p.avFrame2)
            {
                av_frame_free(&p.avFrame2);
            }
            if (p.avFrame)
            {
                av_frame_free(&p.avFrame);
            }
            if (p.avPacket)
            {
                av_free_packet(p.avPacket);
            }
            if (p.avFormatContext->pb)
            {
                avio_closep(&p.avFormatContext->pb);
            }
            if (p.avFormatContext)
            {
                avformat_free_context(p.avFormatContext);
            }
        }

        std::shared_ptr<Write> Write::create(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::writeVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            TLR_PRIVATE_P();

            const auto& info = image->getInfo();
            av_image_fill_arrays(
                p.avFrame2->data,
                p.avFrame2->linesize,
                image->getData(),
                p.avPixelFormatIn,
                info.size.w,
                info.size.h,
                info.layout.alignment);

            // Flip the image vertically.
            switch (info.pixelType)
            {
            case imaging::PixelType::L_U8:
            case imaging::PixelType::RGB_U8:
            case imaging::PixelType::RGBA_U8:
            {
                const uint8_t channelCount = imaging::getChannelCount(info.pixelType);
                for (uint8_t i = 0; i < channelCount; i++)
                {
                    p.avFrame2->data[i] += p.avFrame2->linesize[i] * (info.size.h - 1);
                    p.avFrame2->linesize[i] = -p.avFrame2->linesize[i];
                }
                break;
            }
            case imaging::PixelType::YUV_420P:
                //! \bug How do we flip YUV data?
                throw std::runtime_error(string::Format("{0}: Incompatible pixel type").arg(p.fileName));
                break;
            }

            sws_scale(
                p.swsContext,
                (uint8_t const* const*)p.avFrame2->data,
                p.avFrame2->linesize,
                0,
                p.avVideoStream->codecpar->height,
                p.avFrame->data,
                p.avFrame->linesize);

            const auto timeRational = time::toRational(time.rate());
            p.avFrame->pts = av_rescale_q(
                time.value(),
                { timeRational.second, timeRational.first },
                p.avVideoStream->time_base);
            _encodeVideo(p.avFrame);
        }

        void Write::_encodeVideo(AVFrame* frame)
        {
            TLR_PRIVATE_P();

            int r = avcodec_send_frame(p.avVideoStream->codec, frame);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: Cannot write frame").arg(p.fileName));
            }

            while (r >= 0)
            {
                r = avcodec_receive_packet(p.avVideoStream->codec, p.avPacket);
                if (r == AVERROR(EAGAIN) || r == AVERROR_EOF)
                {
                    return;
                }
                else if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot write frame").arg(p.fileName));
                }
                r = av_interleaved_write_frame(p.avFormatContext, p.avPacket);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot write frame").arg(p.fileName));
                }
                av_packet_unref(p.avPacket);
            }
        }
    }
}
