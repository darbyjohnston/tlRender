// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpeg.h>

#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

namespace tl
{
    namespace ffmpeg
    {
        struct Write::Private
        {
            std::string fileName;
            AVFormatContext* avFormatContext = nullptr;
            AVCodecContext* avCodecContext = nullptr;
            AVStream* avVideoStream = nullptr;
            AVPacket* avPacket = nullptr;
            AVFrame* avFrame = nullptr;
            AVPixelFormat avPixelFormatIn = AV_PIX_FMT_NONE;
            AVFrame* avFrame2 = nullptr;
            SwsContext* swsContext = nullptr;
            bool opened = false;
        };

        void Write::_init(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);

            TLRENDER_P();

            p.fileName = path.get();
            if (info.video.empty())
            {
                throw std::runtime_error(string::Format("{0}: No video").arg(p.fileName));
            }

            AVCodecID avCodecID = AV_CODEC_ID_MPEG4;
            Profile profile = Profile::None;
            int avProfile = FF_PROFILE_UNKNOWN;
            auto option = options.find("FFmpeg/WriteProfile");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> profile;
            }
            switch (profile)
            {
            case Profile::H264:
                avCodecID = AV_CODEC_ID_H264;
                avProfile = FF_PROFILE_H264_HIGH;
                break;
            case Profile::ProRes:
                avCodecID = AV_CODEC_ID_PRORES;
                avProfile = FF_PROFILE_PRORES_STANDARD;
                break;
            case Profile::ProRes_Proxy:
                avCodecID = AV_CODEC_ID_PRORES;
                avProfile = FF_PROFILE_PRORES_PROXY;
                break;
            case Profile::ProRes_LT:
                avCodecID = AV_CODEC_ID_PRORES;
                avProfile = FF_PROFILE_PRORES_LT;
                break;
            case Profile::ProRes_HQ:
                avCodecID = AV_CODEC_ID_PRORES;
                avProfile = FF_PROFILE_PRORES_HQ;
                break;
            case Profile::ProRes_4444:
                avCodecID = AV_CODEC_ID_PRORES;
                avProfile = FF_PROFILE_PRORES_4444;
                break;
            case Profile::ProRes_XQ:
                avCodecID = AV_CODEC_ID_PRORES;
                avProfile = FF_PROFILE_PRORES_XQ;
                break;
            default: break;
            }

            int r = avformat_alloc_output_context2(&p.avFormatContext, NULL, NULL, p.fileName.c_str());
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }
            const AVCodec* avCodec = avcodec_find_encoder(avCodecID);
            if (!avCodec)
            {
                throw std::runtime_error(string::Format("{0}: Cannot find encoder").arg(p.fileName));
            }
            p.avCodecContext = avcodec_alloc_context3(avCodec);
            if (!p.avCodecContext)
            {
                throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(p.fileName));
            }
            p.avVideoStream = avformat_new_stream(p.avFormatContext, avCodec);
            if (!p.avVideoStream)
            {
                throw std::runtime_error(string::Format("{0}: Cannot allocate stream").arg(p.fileName));
            }
            if (!avCodec->pix_fmts)
            {
                throw std::runtime_error(string::Format("{0}: No pixel formats available").arg(p.fileName));
            }

            p.avCodecContext->codec_id = avCodec->id;
            p.avCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            const auto& videoInfo = info.video[0];
            p.avCodecContext->width = videoInfo.size.w;
            p.avCodecContext->height = videoInfo.size.h;
            p.avCodecContext->sample_aspect_ratio = AVRational({ 1, 1 });
            p.avCodecContext->pix_fmt = avCodec->pix_fmts[0];
            const auto rational = time::toRational(info.videoTime.duration().rate());
            p.avCodecContext->time_base = { rational.second, rational.first };
            p.avCodecContext->framerate = { rational.first, rational.second };
            p.avCodecContext->profile = avProfile;
            if (p.avFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            {
                p.avCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            p.avCodecContext->thread_count = 0;
            p.avCodecContext->thread_type = FF_THREAD_FRAME;

            r = avcodec_open2(p.avCodecContext, avCodec, NULL);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }

            r = avcodec_parameters_from_context(p.avVideoStream->codecpar, p.avCodecContext);
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

            //av_dump_format(p.avFormatContext, 0, p.fileName.c_str(), 1);

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
            if (!p.avPacket)
            {
                throw std::runtime_error(string::Format("{0}: Cannot allocate packet").arg(p.fileName));
            }

            p.avFrame = av_frame_alloc();
            if (!p.avFrame)
            {
                throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(p.fileName));
            }
            p.avFrame->format = p.avVideoStream->codecpar->format;
            p.avFrame->width = p.avVideoStream->codecpar->width;
            p.avFrame->height = p.avVideoStream->codecpar->height;
            r = av_frame_get_buffer(p.avFrame, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(p.fileName).arg(getErrorLabel(r)));
            }

            p.avFrame2 = av_frame_alloc();
            if (!p.avFrame2)
            {
                throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(p.fileName));
            }
            switch (videoInfo.pixelType)
            {
            case image::PixelType::L_U8:     p.avPixelFormatIn = AV_PIX_FMT_GRAY8;  break;
            case image::PixelType::RGB_U8:   p.avPixelFormatIn = AV_PIX_FMT_RGB24;  break;
            case image::PixelType::RGBA_U8:  p.avPixelFormatIn = AV_PIX_FMT_RGBA;   break;
            case image::PixelType::L_U16:    p.avPixelFormatIn = AV_PIX_FMT_GRAY16; break;
            case image::PixelType::RGB_U16:  p.avPixelFormatIn = AV_PIX_FMT_RGB48;  break;
            case image::PixelType::RGBA_U16: p.avPixelFormatIn = AV_PIX_FMT_RGBA64; break;
            default:
                throw std::runtime_error(string::Format("{0}: Incompatible pixel type").arg(p.fileName));
                break;
            }
            /*p.swsContext = sws_getContext(
                videoInfo.size.w,
                videoInfo.size.h,
                p.avPixelFormatIn,
                videoInfo.size.w,
                videoInfo.size.h,
                p.avCodecContext->pix_fmt,
                swsScaleFlags,
                0,
                0,
                0);*/
            p.swsContext = sws_alloc_context();
            if (!p.swsContext)
            {
                throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(p.fileName));
            }
            av_opt_set_defaults(p.swsContext);
            r = av_opt_set_int(p.swsContext, "srcw", videoInfo.size.w, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(p.swsContext, "srch", videoInfo.size.h, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(p.swsContext, "src_format", p.avPixelFormatIn, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(p.swsContext, "dstw", videoInfo.size.w, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(p.swsContext, "dsth", videoInfo.size.h, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(p.swsContext, "dst_format", p.avCodecContext->pix_fmt, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(p.swsContext, "sws_flags", swsScaleFlags, AV_OPT_SEARCH_CHILDREN);
            r = av_opt_set_int(p.swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
            r = sws_init_context(p.swsContext, nullptr, nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: Cannot initialize sws context").arg(p.fileName));
            }

            p.opened = true;
        }

        Write::Write() :
            _p(new Private)
        {}

        Write::~Write()
        {
            TLRENDER_P();

            if (p.opened)
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
                av_packet_free(&p.avPacket);
            }
            if (p.avCodecContext)
            {
                avcodec_free_context(&p.avCodecContext);
            }
            if (p.avFormatContext && p.avFormatContext->pb)
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
            const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::writeVideo(
            const otime::RationalTime& time,
            const std::shared_ptr<image::Image>& image,
            const io::Options&)
        {
            TLRENDER_P();

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
            case image::PixelType::L_U8:
            case image::PixelType::L_U16:
            case image::PixelType::RGB_U8:
            case image::PixelType::RGB_U16:
            case image::PixelType::RGBA_U8:
            case image::PixelType::RGBA_U16:
            {
                const size_t channelCount = image::getChannelCount(info.pixelType);
                for (size_t i = 0; i < channelCount; i++)
                {
                    p.avFrame2->data[i] += p.avFrame2->linesize[i] * (info.size.h - 1);
                    p.avFrame2->linesize[i] = -p.avFrame2->linesize[i];
                }
                break;
            }
            case image::PixelType::YUV_420P_U8:
            case image::PixelType::YUV_422P_U8:
            case image::PixelType::YUV_444P_U8:
            case image::PixelType::YUV_420P_U16:
            case image::PixelType::YUV_422P_U16:
            case image::PixelType::YUV_444P_U16:
                //! \bug How do we flip YUV data?
                throw std::runtime_error(string::Format("{0}: Incompatible pixel type").arg(p.fileName));
                break;
            default: break;
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
            TLRENDER_P();

            int r = avcodec_send_frame(p.avCodecContext, frame);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: Cannot write frame").arg(p.fileName));
            }

            while (r >= 0)
            {
                r = avcodec_receive_packet(p.avCodecContext, p.avPacket);
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
