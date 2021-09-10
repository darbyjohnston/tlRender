// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FFmpeg.h>

#include <tlrCore/Assert.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

extern "C"
{
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>

} // extern "C"

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <queue>
#include <list>
#include <map>
#include <mutex>
#include <thread>

namespace tlr
{
    namespace ffmpeg
    {
        struct Read::Private
        {
            int decodeVideo(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&);
            void copyVideo(const std::shared_ptr<imaging::Image>&);

            avio::Info info;
            std::promise<avio::Info> infoPromise;
            struct VideoFrameRequest
            {
                VideoFrameRequest() {}
                VideoFrameRequest(VideoFrameRequest&&) = default;

                otime::RationalTime time = time::invalidTime;
                std::shared_ptr<imaging::Image> image;
                std::promise<avio::VideoFrame> promise;
            };
            std::list<VideoFrameRequest> videoFrameRequests;
            std::condition_variable requestCV;
            otime::RationalTime currentTime = time::invalidTime;
            std::list<std::shared_ptr<imaging::Image> > imageBuffer;
            bool eof = false;

            AVFormatContext* avFormatContext = nullptr;
            int avVideoStream = -1;
            std::map<int, AVCodecParameters*> avCodecParameters;
            std::map<int, AVCodecContext*> avCodecContext;
            AVFrame* avFrame = nullptr;
            AVFrame* avFrame2 = nullptr;
            SwsContext* swsContext = nullptr;

            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running;
            bool stopped = false;
            size_t threadCount = ffmpeg::threadCount;
        };

        void Read::_init(
            const file::Path& path,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IRead::_init(path, options, logSystem);

            TLR_PRIVATE_P();

            const auto i = options.find("ffmpeg/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.threadCount;
            }

            p.running = true;
            p.thread = std::thread(
                [this, path]
                {
                    TLR_PRIVATE_P();
                    try
                    {
                        _open(path.get());
                        try
                        {
                            _run();
                        }
                        catch (const std::exception&)
                        {}
                    }
                    catch (const std::exception& e)
                    {
                        p.infoPromise.set_value(avio::Info());
                    }
                                        
                    std::list<Private::VideoFrameRequest> videoFrameRequestsCleanup;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.stopped = true;
                        videoFrameRequestsCleanup.swap(p.videoFrameRequests);
                    }
                    for (auto& i : videoFrameRequestsCleanup)
                    {
                        i.promise.set_value(avio::VideoFrame());
                    }
                    _close();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            TLR_PRIVATE_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const avio::Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, options, logSystem);
            return out;
        }

        std::future<avio::Info> Read::getInfo()
        {
            return _p->infoPromise.get_future();
        }

        std::future<avio::VideoFrame> Read::readVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            TLR_PRIVATE_P();
            Private::VideoFrameRequest request;
            request.time = time;
            request.image = image;
            auto future = request.promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.videoFrameRequests.push_back(std::move(request));
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request.promise.set_value(avio::VideoFrame());
            }
            return future;
        }

        bool Read::hasVideoFrames()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return !p.videoFrameRequests.empty();
        }

        void Read::cancelVideoFrames()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.videoFrameRequests.clear();
        }

        void Read::stop()
        {
            _p->running = false;
        }

        bool Read::hasStopped() const
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.stopped;
        }

        void Read::_open(const std::string& fileName)
        {
            TLR_PRIVATE_P();
            int r = avformat_open_input(
                &p.avFormatContext,
                fileName.c_str(),
                nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            r = avformat_find_stream_info(p.avFormatContext, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            //av_dump_format(p.avFormatContext, 0, fileName.c_str(), 0);

            for (unsigned int i = 0; i < p.avFormatContext->nb_streams; ++i)
            {
                if (-1 == p.avVideoStream && p.avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    p.avVideoStream = i;
                }
            }
            if (-1 == p.avVideoStream)
            {
                throw std::runtime_error(string::Format("{0}: No video stream found").arg(fileName));
            }

            p.avFrame = av_frame_alloc();

            std::size_t sequenceSize = 0;
            if (p.avVideoStream != -1)
            {
                auto avVideoStream = p.avFormatContext->streams[p.avVideoStream];
                auto avVideoCodecParameters = avVideoStream->codecpar;
                auto avVideoCodec = avcodec_find_decoder(avVideoCodecParameters->codec_id);
                if (!avVideoCodec)
                {
                    throw std::runtime_error(string::Format("{0}: No video codec found").arg(fileName));
                }
                p.avCodecParameters[p.avVideoStream] = avcodec_parameters_alloc();
                r = avcodec_parameters_copy(p.avCodecParameters[p.avVideoStream], avVideoCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.avCodecContext[p.avVideoStream] = avcodec_alloc_context3(avVideoCodec);
                r = avcodec_parameters_to_context(p.avCodecContext[p.avVideoStream], p.avCodecParameters[p.avVideoStream]);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.avCodecContext[p.avVideoStream]->thread_count = p.threadCount;
                p.avCodecContext[p.avVideoStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(p.avCodecContext[p.avVideoStream], avVideoCodec, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }

                imaging::Info videoInfo;
                videoInfo.size.w = p.avCodecParameters[p.avVideoStream]->width;
                videoInfo.size.h = p.avCodecParameters[p.avVideoStream]->height;
                videoInfo.layout.mirror.y = true;

                const AVPixelFormat avPixelFormat = static_cast<AVPixelFormat>(p.avCodecParameters[p.avVideoStream]->format);
                switch (avPixelFormat)
                {
                case AV_PIX_FMT_YUV420P:
                    videoInfo.pixelType = imaging::PixelType::YUV_420P;
                    break;
                case AV_PIX_FMT_RGB24:
                    videoInfo.pixelType = imaging::PixelType::RGB_U8;
                    break;
                case AV_PIX_FMT_GRAY8:
                    videoInfo.pixelType = imaging::PixelType::L_U8;
                    break;
                case AV_PIX_FMT_RGBA:
                    videoInfo.pixelType = imaging::PixelType::RGBA_U8;
                    break;
                default:
                    videoInfo.pixelType = imaging::PixelType::YUV_420P;
                    p.avFrame2 = av_frame_alloc();
                    p.swsContext = sws_getContext(
                        p.avCodecParameters[p.avVideoStream]->width,
                        p.avCodecParameters[p.avVideoStream]->height,
                        avPixelFormat,
                        p.avCodecParameters[p.avVideoStream]->width,
                        p.avCodecParameters[p.avVideoStream]->height,
                        AV_PIX_FMT_YUV420P,
                        swsScaleFlags,
                        0,
                        0,
                        0);
                    break;
                }

                if (avVideoStream->duration != AV_NOPTS_VALUE)
                {
                    sequenceSize = av_rescale_q(
                        avVideoStream->duration,
                        avVideoStream->time_base,
                        swap(avVideoStream->r_frame_rate));
                }
                else if (p.avFormatContext->duration != AV_NOPTS_VALUE)
                {
                    sequenceSize = av_rescale_q(
                        p.avFormatContext->duration,
                        av_get_time_base_q(),
                        swap(avVideoStream->r_frame_rate));
                }
                p.info.video.push_back(videoInfo);
                const float speed = avVideoStream->r_frame_rate.num / double(avVideoStream->r_frame_rate.den);
                p.info.videoTimeRange = otime::TimeRange(
                    otime::RationalTime(0.0, speed),
                    otime::RationalTime(
                        sequenceSize,
                        avVideoStream->r_frame_rate.num / double(avVideoStream->r_frame_rate.den)));

                p.currentTime = otime::RationalTime(0.0, speed);
            }

            AVDictionaryEntry* tag = nullptr;
            while ((tag = av_dict_get(p.avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
            {
                p.info.tags[tag->key] = tag->value;
            }

            p.infoPromise.set_value(p.info);
        }

        void Read::_run()
        {
            TLR_PRIVATE_P();
            while (p.running)
            {
                Private::VideoFrameRequest request;
                bool requestValid = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.requestCV.wait_for(
                        lock,
                        requestTimeout,
                        [this]
                        {
                            return !_p->videoFrameRequests.empty();
                        });
                    if (!p.videoFrameRequests.empty())
                    {
                        auto& tmp = p.videoFrameRequests.front();
                        request.time = tmp.time;
                        request.image = tmp.image;
                        request.promise = std::move(tmp.promise);
                        p.videoFrameRequests.pop_front();
                        requestValid = true;
                    }
                }
                if (requestValid)
                {
                    //std::cout << "----------" << std::endl;
                    //std::cout << "request: " << request.time << std::endl;
                    //std::cout << "current time: " << p.currentTime << std::endl;

                    if (request.time != p.currentTime)
                    {
                        //std::cout << "seek: " << request.time << std::endl;
                        p.currentTime = request.time;
                        p.imageBuffer.clear();
                        p.eof = false;
                        int64_t t = 0;
                        int stream = -1;
                        if (p.avVideoStream != -1)
                        {
                            avcodec_flush_buffers(p.avCodecContext[p.avVideoStream]);
                            stream = p.avVideoStream;
                            t = av_rescale_q(
                                request.time.value(),
                                swap(p.avFormatContext->streams[p.avVideoStream]->r_frame_rate),
                                p.avFormatContext->streams[p.avVideoStream]->time_base);
                        }
                        if (av_seek_frame(
                            p.avFormatContext,
                            stream,
                            t,
                            AVSEEK_FLAG_BACKWARD) < 0)
                        {
                            //! \todo How should this be handled?
                        }
                    }

                    if (p.imageBuffer.empty())
                    {
                        int decoding = 0;
                        AVPacket packet;
                        while (0 == decoding)
                        {
                            if (!p.eof)
                            {
                                decoding = av_read_frame(p.avFormatContext, &packet);
                                if (AVERROR_EOF == decoding)
                                {
                                    p.eof = true;
                                    decoding = 0;
                                }
                                else if (decoding < 0)
                                {
                                    //! \todo How should this be handled?
                                    break;
                                }
                            }
                            if (p.eof ? true : (p.avVideoStream == packet.stream_index))
                            {
                                decoding = avcodec_send_packet(
                                    p.avCodecContext[p.avVideoStream],
                                    p.eof ? nullptr : &packet);
                                if (AVERROR_EOF == decoding)
                                {
                                    decoding = 0;
                                }
                                else if (decoding < 0)
                                {
                                    //! \todo How should this be handled?
                                    break;
                                }
                                decoding = p.decodeVideo(request.time, request.image);
                                request.image = nullptr;
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
                            }
                            if (!p.eof)
                            {
                                av_packet_unref(&packet);
                            }
                        }
                    }

                    avio::VideoFrame videoFrame;
                    if (!p.imageBuffer.empty())
                    {
                        videoFrame.image = p.imageBuffer.front();
                        videoFrame.time = request.time;
                        p.imageBuffer.pop_front();
                    }
                    request.promise.set_value(videoFrame);

                    p.currentTime = request.time + otime::RationalTime(1.0, p.currentTime.rate());
                }
            }
        }

        void Read::_close()
        {
            TLR_PRIVATE_P();
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
            for (auto i : p.avCodecContext)
            {
                avcodec_close(i.second);
                avcodec_free_context(&i.second);
            }
            for (auto i : p.avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (p.avFormatContext)
            {
                avformat_close_input(&p.avFormatContext);
            }
        }

        int Read::Private::decodeVideo(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            int out = 0;
            while (0 == out)
            {
                out = avcodec_receive_frame(avCodecContext[avVideoStream], avFrame);
                if (out < 0)
                {
                    return out;
                }

                const auto& videoInfo = info.video[0];
                const auto t = otime::RationalTime(
                    av_rescale_q(
                        avFrame->pts,
                        avFormatContext->streams[avVideoStream]->time_base,
                        swap(avFormatContext->streams[avVideoStream]->r_frame_rate)),
                    info.videoTimeRange.duration().rate());
                if (t >= time)
                {
                    //std::cout << "frame: " << t << std::endl;
                    auto tmp = image && image->getInfo() == info.video[0] ? image : imaging::Image::create(videoInfo);
                    tmp->setTags(info.tags);
                    copyVideo(tmp);
                    imageBuffer.push_back(tmp);

                    out = 1;
                }
            }
            return out;
        }

        void Read::Private::copyVideo(const std::shared_ptr<imaging::Image>& image)
        {
            const auto& info = image->getInfo();
            const std::size_t w = info.size.w;
            const std::size_t h = info.size.h;
            const AVPixelFormat avPixelFormat = static_cast<AVPixelFormat>(avCodecParameters[avVideoStream]->format);
            switch (avPixelFormat)
            {
            case AV_PIX_FMT_YUV420P:
            {
                const std::size_t w2 = w / 2;
                const std::size_t h2 = h / 2;
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        image->getData() + w * i,
                        avFrame->data[0] + avFrame->linesize[0] * i,
                        w);
                }
                for (std::size_t i = 0; i < h2; ++i)
                {
                    std::memcpy(
                        image->getData() + (w * h) + w2 * i,
                        avFrame->data[1] + avFrame->linesize[1] * i,
                        w2);
                    std::memcpy(
                        image->getData() + (w * h) + (w2 * h2) + w2 * i,
                        avFrame->data[2] + avFrame->linesize[2] * i,
                        w2);
                }
                break;
            }
            case AV_PIX_FMT_RGB24:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        image->getData() + w * 3 * i,
                        avFrame->data[0] + avFrame->linesize[0] * 3 * i,
                        w * 3);
                }
                break;
            case AV_PIX_FMT_GRAY8:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        image->getData() + w * i,
                        avFrame->data[0] + avFrame->linesize[0] * i,
                        w);
                }
                break;
            case AV_PIX_FMT_RGBA:
                for (std::size_t i = 0; i < h; ++i)
                {
                    std::memcpy(
                        image->getData() + w * 4 * i,
                        avFrame->data[0] + avFrame->linesize[0] * 4 * i,
                        w * 4);
                }
                break;
            default:
                av_image_fill_arrays(
                    avFrame2->data,
                    avFrame2->linesize,
                    image->getData(),
                    AV_PIX_FMT_YUV420P,
                    w,
                    h,
                    1);
                sws_scale(
                    swsContext,
                    (uint8_t const* const*)avFrame->data,
                    avFrame->linesize,
                    0,
                    avCodecParameters[avVideoStream]->height,
                    avFrame2->data,
                    avFrame2->linesize);
                break;
            }
        }
    }
}
