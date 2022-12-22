// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

} // extern "C"

namespace tl
{
    namespace ffmpeg
    {
        void Read::_openVideo(const std::string& fileName)
        {
            TLRENDER_P();

            if (!_memory.empty())
            {
                p.video.avFormatContext = avformat_alloc_context();
                if (!p.video.avFormatContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate format context").arg(fileName));
                }

                p.video.avIOBufferData = AVIOBufferData(_memory[0].p, _memory[0].size);
                p.video.avIOContextBuffer = static_cast<uint8_t*>(av_malloc(avIOContextBufferSize));
                p.video.avIOContext = avio_alloc_context(
                    p.video.avIOContextBuffer,
                    avIOContextBufferSize,
                    0,
                    &p.video.avIOBufferData,
                    &avIOBufferRead,
                    nullptr,
                    &avIOBufferSeek);
                if (!p.video.avIOContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate I/O context").arg(fileName));
                }

                p.video.avFormatContext->pb = p.video.avIOContext;
            }

            int r = avformat_open_input(
                &p.video.avFormatContext,
                !p.video.avFormatContext ? fileName.c_str() : nullptr,
                nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }

            r = avformat_find_stream_info(p.video.avFormatContext, nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            for (unsigned int i = 0; i < p.video.avFormatContext->nb_streams; ++i)
            {
                //av_dump_format(p.video.avFormatContext, 0, fileName.c_str(), 0);

                if (AVMEDIA_TYPE_VIDEO == p.video.avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT == p.video.avFormatContext->streams[i]->disposition)
                {
                    p.video.avStream = i;
                    break;
                }
            }
            if (-1 == p.video.avStream)
            {
                for (unsigned int i = 0; i < p.video.avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_VIDEO == p.video.avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        p.video.avStream = i;
                        break;
                    }
                }
            }
            int dataStream = -1;
            for (unsigned int i = 0; i < p.video.avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_DATA == p.video.avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT == p.video.avFormatContext->streams[i]->disposition)
                {
                    dataStream = i;
                    break;
                }
            }
            if (-1 == dataStream)
            {
                for (unsigned int i = 0; i < p.video.avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_DATA == p.video.avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        dataStream = i;
                        break;
                    }
                }
            }
            std::string timecode;
            if (dataStream != -1)
            {
                AVDictionaryEntry* tag = nullptr;
                while ((tag = av_dict_get(
                    p.video.avFormatContext->streams[dataStream]->metadata,
                    "",
                    tag,
                    AV_DICT_IGNORE_SUFFIX)))
                {
                    if (string::compareNoCase(tag->key, "timecode"))
                    {
                        timecode = tag->value;
                        break;
                    }
                }
            }
            if (p.video.avStream != -1)
            {
                //av_dump_format(p.video.avFormatContext, p.video.avStream, fileName.c_str(), 0);

                auto avVideoStream = p.video.avFormatContext->streams[p.video.avStream];
                auto avVideoCodecParameters = avVideoStream->codecpar;
                auto avVideoCodec = avcodec_find_decoder(avVideoCodecParameters->codec_id);
                if (!avVideoCodec)
                {
                    throw std::runtime_error(string::Format("{0}: No video codec found").arg(fileName));
                }
                p.video.avCodecParameters[p.video.avStream] = avcodec_parameters_alloc();
                if (!p.video.avCodecParameters[p.video.avStream])
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate parameters").arg(fileName));
                }
                r = avcodec_parameters_copy(p.video.avCodecParameters[p.video.avStream], avVideoCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.video.avCodecContext[p.video.avStream] = avcodec_alloc_context3(avVideoCodec);
                if (!p.video.avCodecParameters[p.video.avStream])
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(fileName));
                }
                r = avcodec_parameters_to_context(p.video.avCodecContext[p.video.avStream], p.video.avCodecParameters[p.video.avStream]);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.video.avCodecContext[p.video.avStream]->thread_count = p.options.threadCount;
                p.video.avCodecContext[p.video.avStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(p.video.avCodecContext[p.video.avStream], avVideoCodec, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }

                imaging::Info videoInfo;
                videoInfo.size.w = p.video.avCodecParameters[p.video.avStream]->width;
                videoInfo.size.h = p.video.avCodecParameters[p.video.avStream]->height;
                if (p.video.avCodecParameters[p.video.avStream]->sample_aspect_ratio.den > 0 &&
                    p.video.avCodecParameters[p.video.avStream]->sample_aspect_ratio.num > 0)
                {
                    videoInfo.size.pixelAspectRatio = av_q2d(p.video.avCodecParameters[p.video.avStream]->sample_aspect_ratio);
                }
                videoInfo.layout.mirror.y = true;

                p.video.avInputPixelFormat = static_cast<AVPixelFormat>(
                    p.video.avCodecParameters[p.video.avStream]->format);
                switch (p.video.avInputPixelFormat)
                {
                case AV_PIX_FMT_RGB24:
                    p.video.avOutputPixelFormat = p.video.avInputPixelFormat;
                    videoInfo.pixelType = imaging::PixelType::RGB_U8;
                    break;
                case AV_PIX_FMT_GRAY8:
                    p.video.avOutputPixelFormat = p.video.avInputPixelFormat;
                    videoInfo.pixelType = imaging::PixelType::L_U8;
                    break;
                case AV_PIX_FMT_RGBA:
                    p.video.avOutputPixelFormat = p.video.avInputPixelFormat;
                    videoInfo.pixelType = imaging::PixelType::RGBA_U8;
                    break;
                case AV_PIX_FMT_YUV420P:
                    if (p.options.yuvToRGBConversion)
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        videoInfo.pixelType = imaging::PixelType::RGB_U8;
                    }
                    else
                    {
                        p.video.avOutputPixelFormat = p.video.avInputPixelFormat;
                        videoInfo.pixelType = imaging::PixelType::YUV_420P_U8;
                    }
                    break;
                case AV_PIX_FMT_YUV422P:
                    if (p.options.yuvToRGBConversion)
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        videoInfo.pixelType = imaging::PixelType::RGB_U8;
                    }
                    else
                    {
                        p.video.avOutputPixelFormat = p.video.avInputPixelFormat;
                        videoInfo.pixelType = imaging::PixelType::YUV_422P_U8;
                    }
                    break;
                case AV_PIX_FMT_YUV444P:
                    if (p.options.yuvToRGBConversion)
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        videoInfo.pixelType = imaging::PixelType::RGB_U8;
                    }
                    else
                    {
                        p.video.avOutputPixelFormat = p.video.avInputPixelFormat;
                        videoInfo.pixelType = imaging::PixelType::YUV_444P_U8;
                    }
                    break;
                case AV_PIX_FMT_YUV420P10BE:
                case AV_PIX_FMT_YUV420P10LE:
                case AV_PIX_FMT_YUV420P12BE:
                case AV_PIX_FMT_YUV420P12LE:
                case AV_PIX_FMT_YUV420P16BE:
                case AV_PIX_FMT_YUV420P16LE:
                    if (p.options.yuvToRGBConversion)
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        videoInfo.pixelType = imaging::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the videoInfo.layout.endian field instead of
                        //! converting endianness.
                        p.video.avOutputPixelFormat = AV_PIX_FMT_YUV420P16LE;
                        videoInfo.pixelType = imaging::PixelType::YUV_420P_U16;
                    }
                    break;
                case AV_PIX_FMT_YUV422P10BE:
                case AV_PIX_FMT_YUV422P10LE:
                case AV_PIX_FMT_YUV422P12BE:
                case AV_PIX_FMT_YUV422P12LE:
                case AV_PIX_FMT_YUV422P16BE:
                case AV_PIX_FMT_YUV422P16LE:
                    if (p.options.yuvToRGBConversion)
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        videoInfo.pixelType = imaging::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the videoInfo.layout.endian field instead of
                        //! converting endianness.
                        p.video.avOutputPixelFormat = AV_PIX_FMT_YUV422P16LE;
                        videoInfo.pixelType = imaging::PixelType::YUV_422P_U16;
                    }
                    break;
                case AV_PIX_FMT_YUV444P10BE:
                case AV_PIX_FMT_YUV444P10LE:
                case AV_PIX_FMT_YUV444P12BE:
                case AV_PIX_FMT_YUV444P12LE:
                case AV_PIX_FMT_YUV444P16BE:
                case AV_PIX_FMT_YUV444P16LE:
                case AV_PIX_FMT_YUVA444P10BE:
                case AV_PIX_FMT_YUVA444P10LE:
                case AV_PIX_FMT_YUVA444P12BE:
                case AV_PIX_FMT_YUVA444P12LE:
                case AV_PIX_FMT_YUVA444P16BE:
                case AV_PIX_FMT_YUVA444P16LE:
                    if (p.options.yuvToRGBConversion)
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_RGB48;
                        videoInfo.pixelType = imaging::PixelType::RGB_U16;
                    }
                    else
                    {
                        //! \todo Use the videoInfo.layout.endian field instead of
                        //! converting endianness.
                        p.video.avOutputPixelFormat = AV_PIX_FMT_YUV444P16LE;
                        videoInfo.pixelType = imaging::PixelType::YUV_444P_U16;
                    }
                    break;
                default:
                    if (p.options.yuvToRGBConversion)
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_RGB24;
                        videoInfo.pixelType = imaging::PixelType::RGB_U8;
                    }
                    else
                    {
                        p.video.avOutputPixelFormat = AV_PIX_FMT_YUV420P;
                        videoInfo.pixelType = imaging::PixelType::YUV_420P_U8;
                    }
                    break;
                }
                if (p.video.avCodecContext[p.video.avStream]->color_range != AVCOL_RANGE_JPEG)
                {
                    videoInfo.videoLevels = imaging::VideoLevels::LegalRange;
                }
                switch (p.video.avCodecParameters[p.video.avStream]->color_space)
                {
                case AVCOL_PRI_BT2020:
                    videoInfo.yuvCoefficients = imaging::YUVCoefficients::BT2020;
                    break;
                default: break;
                }

                std::size_t sequenceSize = 0;
                if (avVideoStream->duration != AV_NOPTS_VALUE)
                {
                    sequenceSize = av_rescale_q(
                        avVideoStream->duration,
                        avVideoStream->time_base,
                        swap(avVideoStream->r_frame_rate));
                }
                else if (p.video.avFormatContext->duration != AV_NOPTS_VALUE)
                {
                    sequenceSize = av_rescale_q(
                        p.video.avFormatContext->duration,
                        av_get_time_base_q(),
                        swap(avVideoStream->r_frame_rate));
                }
                p.info.video.push_back(videoInfo);

                const double speed = avVideoStream->r_frame_rate.num / double(avVideoStream->r_frame_rate.den);

                imaging::Tags tags;
                AVDictionaryEntry* tag = nullptr;
                while ((tag = av_dict_get(p.video.avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                {
                    const std::string key(tag->key);
                    const std::string value(tag->value);
                    tags[key] = value;
                    if (string::compareNoCase(key, "timecode"))
                    {
                        timecode = tag->value;
                    }
                }

                otime::RationalTime startTime(0.0, speed);
                if (!timecode.empty())
                {
                    otime::ErrorStatus errorStatus;
                    const otime::RationalTime time = otime::RationalTime::from_timecode(
                        timecode,
                        speed,
                        &errorStatus);
                    if (!otime::is_error(errorStatus))
                    {
                        startTime = time::floor(time);
                        //std::cout << "start time: " << startTime << std::endl;
                    }
                }
                p.info.videoTime = otime::TimeRange(
                    startTime,
                    otime::RationalTime(sequenceSize, speed));

                p.currentVideoTime = p.info.videoTime.start_time();

                for (const auto& i : tags)
                {
                    p.info.tags[i.first] = i.second;
                }
                {
                    std::stringstream ss;
                    ss << videoInfo.size.w << " " << videoInfo.size.h;
                    p.info.tags["Video Resolution"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << videoInfo.size.pixelAspectRatio;
                    p.info.tags["Video Pixel Aspect Ratio"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << videoInfo.pixelType;
                    p.info.tags["Video Pixel Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << videoInfo.videoLevels;
                    p.info.tags["Video Levels"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << p.info.videoTime.start_time().to_timecode();
                    p.info.tags["Video Start Time"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << p.info.videoTime.duration().to_timecode();
                    p.info.tags["Video Duration"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << p.info.videoTime.start_time().rate() << " FPS";
                    p.info.tags["Video Speed"] = ss.str();
                }
            }
        }

        void Read::Private::startVideo(const std::string& fileName)
        {
            if (video.avStream != -1)
            {
                video.avFrame = av_frame_alloc();
                if (!video.avFrame)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(fileName));
                }

                switch (video.avInputPixelFormat)
                {
                case AV_PIX_FMT_RGB24:
                case AV_PIX_FMT_GRAY8:
                case AV_PIX_FMT_RGBA:
                case AV_PIX_FMT_YUV420P:
                    break;
                default:
                {
                    video.avFrame2 = av_frame_alloc();
                    if (!video.avFrame2)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(fileName));
                    }

                    /*video.swsContext = sws_getContext(
                        video.avCodecParameters[video.avStream]->width,
                        video.avCodecParameters[video.avStream]->height,
                        video.avInputPixelFormat,
                        video.avCodecParameters[video.avStream]->width,
                        video.avCodecParameters[video.avStream]->height,
                        video.avOutputPixelFormat,
                        swsScaleFlags,
                        0,
                        0,
                        0);
                    if (!video.swsContext)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot get context").arg(_path.get()));
                    }*/
                    video.swsContext = sws_alloc_context();
                    if (!video.swsContext)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(fileName));
                    }
                    av_opt_set_defaults(video.swsContext);
                    int r = av_opt_set_int(video.swsContext, "srcw", video.avCodecParameters[video.avStream]->width, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(video.swsContext, "srch", video.avCodecParameters[video.avStream]->height, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(video.swsContext, "src_format", video.avInputPixelFormat, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(video.swsContext, "dstw", video.avCodecParameters[video.avStream]->width, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(video.swsContext, "dsth", video.avCodecParameters[video.avStream]->height, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(video.swsContext, "dst_format", video.avOutputPixelFormat, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(video.swsContext, "sws_flags", swsScaleFlags, AV_OPT_SEARCH_CHILDREN);
                    r = av_opt_set_int(video.swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
                    r = sws_init_context(video.swsContext, nullptr, nullptr);
                    if (r < 0)
                    {
                        throw std::runtime_error(string::Format("{0}: Cannot initialize sws context").arg(fileName));
                    }
                    break;
                }
                }
            }
        }

        void Read::Private::processVideo()
        {
            bool seek = false;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (currentVideoRequest)
                {
                    if (!time::compareExact(currentVideoRequest->time, currentVideoTime))
                    {
                        seek = true;
                        currentVideoTime = currentVideoRequest->time;
                    }
                }
            }
            if (seek)
            {
                //std::cout << "video seek: " << currentVideoTime << std::endl;

                if (video.avStream != -1)
                {
                    avcodec_flush_buffers(video.avCodecContext[video.avStream]);

                    if (av_seek_frame(
                        video.avFormatContext,
                        video.avStream,
                        av_rescale_q(
                            currentVideoTime.value() - info.videoTime.start_time().value(),
                            swap(video.avFormatContext->streams[video.avStream]->r_frame_rate),
                            video.avFormatContext->streams[video.avStream]->time_base),
                        AVSEEK_FLAG_BACKWARD) < 0)
                    {
                        //! \todo How should this be handled?
                    }
                }

                video.buffer.clear();
            }

            if (video.avStream != -1 &&
                video.buffer.size() < options.videoBufferSize)
            {
                AVPacket packet;
                av_init_packet(&packet);
                int decoding = 0;
                bool eof = false;
                while (0 == decoding)
                {
                    if (!eof)
                    {
                        decoding = av_read_frame(video.avFormatContext, &packet);
                        if (AVERROR_EOF == decoding)
                        {
                            eof = true;
                            decoding = 0;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                    }
                    if ((eof && video.avStream != -1) || (video.avStream == packet.stream_index))
                    {
                        decoding = avcodec_send_packet(
                            video.avCodecContext[video.avStream],
                            eof ? nullptr : &packet);
                        if (AVERROR_EOF == decoding)
                        {
                            decoding = 0;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                        decoding = decodeVideo();
                        if (AVERROR(EAGAIN) == decoding)
                        {
                            decoding = 0;
                        }
                        else if (AVERROR_EOF == decoding)
                        {
                            break;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                        else if (1 == decoding)
                        {
                            break;
                        }
                    }
                    if (packet.buf)
                    {
                        av_packet_unref(&packet);
                    }
                }
                if (packet.buf)
                {
                    av_packet_unref(&packet);
                }
                //std::cout << "video buffer size: " << video.buffer.size() << std::endl;
            }

            std::shared_ptr<Private::VideoRequest> request;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if ((currentVideoRequest && !video.buffer.empty()) ||
                    (currentVideoRequest && -1 == video.avStream))
                {
                    request = std::move(currentVideoRequest);
                }
            }
            if (request)
            {
                io::VideoData data;
                data.time = request->time;
                if (!video.buffer.empty())
                {
                    data.image = video.buffer.front();
                    video.buffer.pop_front();
                }
                request->promise.set_value(data);

                currentVideoTime += otime::RationalTime(1.0, info.videoTime.duration().rate());
            }
        }

        int Read::Private::decodeVideo()
        {
            int out = 0;
            while (0 == out)
            {
                out = avcodec_receive_frame(video.avCodecContext[video.avStream], video.avFrame);
                if (out < 0)
                {
                    return out;
                }
                const int64_t timestamp = video.avFrame->pts != AV_NOPTS_VALUE ? video.avFrame->pts : video.avFrame->pkt_dts;
                //std::cout << "video timestamp: " << timestamp << std::endl;

                const auto time = otime::RationalTime(
                    info.videoTime.start_time().value() +
                    av_rescale_q(
                        timestamp,
                        video.avFormatContext->streams[video.avStream]->time_base,
                        swap(video.avFormatContext->streams[video.avStream]->r_frame_rate)),
                    info.videoTime.duration().rate());
                //std::cout << "video time: " << time << std::endl;

                if (time >= currentVideoTime)
                {
                    //std::cout << "video time: " << time << std::endl;
                    auto image = imaging::Image::create(info.video[0]);
                    
                    auto tags = info.tags;
                    AVDictionaryEntry* tag = nullptr;
                    while ((tag = av_dict_get(video.avFrame->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                    {
                        tags[tag->key] = tag->value;
                    }
                    imaging::HDRData hdrData;
                    toHDRData(video.avFrame->side_data, video.avFrame->nb_side_data, hdrData);
                    tags["hdr"] = nlohmann::json(hdrData).dump();
                    image->setTags(tags);

                    copyVideo(image);
                    video.buffer.push_back(image);
                    out = 1;
                    break;
                }
            }
            return out;
        }

        void Read::Private::copyVideo(const std::shared_ptr<imaging::Image>& image)
        {
            const auto& info = image->getInfo();
            const std::size_t w = info.size.w;
            const std::size_t h = info.size.h;
            const AVPixelFormat avPixelFormat = static_cast<AVPixelFormat>(video.avCodecParameters[video.avStream]->format);
            uint8_t* const data = image->getData();
            const uint8_t* const data0 = video.avFrame->data[0];
            const int linesize0 = video.avFrame->linesize[0];
            switch (avPixelFormat)
            {
            case AV_PIX_FMT_RGB24:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        data + w * 3 * i,
                        data0 + linesize0 * 3 * i,
                        w * 3);
                }
                break;
            case AV_PIX_FMT_GRAY8:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        data + w * i,
                        data0 + linesize0 * i,
                        w);
                }
                break;
            case AV_PIX_FMT_RGBA:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        data + w * 4 * i,
                        data0 + linesize0 * 4 * i,
                        w * 4);
                }
                break;
            case AV_PIX_FMT_YUV420P:
            {
                const std::size_t w2 = w / 2;
                const std::size_t h2 = h / 2;
                const uint8_t* const data1 = video.avFrame->data[1];
                const uint8_t* const data2 = video.avFrame->data[2];
                const int linesize1 = video.avFrame->linesize[1];
                const int linesize2 = video.avFrame->linesize[2];
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        data + w * i,
                        data0 + linesize0 * i,
                        w);
                }
                for (std::size_t i = 0; i < h2; ++i)
                {
                    std::memcpy(
                        data + (w * h) + w2 * i,
                        data1 + linesize1 * i,
                        w2);
                    std::memcpy(
                        data + (w * h) + (w2 * h2) + w2 * i,
                        data2 + linesize2 * i,
                        w2);
                }
                break;
            }
            default:
            {
                av_image_fill_arrays(
                    video.avFrame2->data,
                    video.avFrame2->linesize,
                    data,
                    video.avOutputPixelFormat,
                    w,
                    h,
                    1);
                sws_scale(
                    video.swsContext,
                    (uint8_t const* const*)video.avFrame->data,
                    video.avFrame->linesize,
                    0,
                    video.avCodecParameters[video.avStream]->height,
                    video.avFrame2->data,
                    video.avFrame2->linesize);
                break;
            }
            }
        }
    }
}
