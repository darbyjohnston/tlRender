// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpeg.h>

#include <tlCore/Assert.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>

} // extern "C"

#include <atomic>
#include <bitset>
#include <condition_variable>
#include <cstring>
#include <queue>
#include <list>
#include <map>
#include <mutex>
#include <thread>

namespace tl
{
    namespace ffmpeg
    {
        struct Read::Private
        {
            io::Info info;
            std::promise<io::Info> infoPromise;

            struct VideoRequest
            {
                otime::RationalTime time = time::invalidTime;
                std::promise<io::VideoData> promise;
            };
            std::list<std::shared_ptr<VideoRequest> > videoRequests;
            otime::RationalTime videoTime = time::invalidTime;

            audio::Info audioConvertInfo;
            struct AudioRequest
            {
                otime::TimeRange time = time::invalidTimeRange;
                std::promise<io::AudioData> promise;
            };
            std::list< std::shared_ptr<AudioRequest> > audioRequests;
            otime::RationalTime audioTime = time::invalidTime;

            std::condition_variable requestCV;

            struct Video
            {
                AVFormatContext* avFormatContext = nullptr;
                int avStream = -1;
                std::map<int, AVCodecParameters*> avCodecParameters;
                std::map<int, AVCodecContext*> avCodecContext;
                AVFrame* avFrame = nullptr;
                AVFrame* avFrame2 = nullptr;
                SwsContext* swsContext = nullptr;
                std::list<std::shared_ptr<imaging::Image> > buffer;
            };
            Video video;

            struct Audio
            {
                AVFormatContext* avFormatContext = nullptr;
                int avStream = -1;
                std::map<int, AVCodecParameters*> avCodecParameters;
                std::map<int, AVCodecContext*> avCodecContext;
                AVFrame* avFrame = nullptr;
                SwrContext* swrContext = nullptr;
                std::list<std::shared_ptr<audio::Audio> > buffer;
            };
            Audio audio;

            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running;
            bool stopped = false;
            size_t threadCount = ffmpeg::threadCount;

            std::chrono::steady_clock::time_point logTimer;

            int decodeVideo();
            void copyVideo(const std::shared_ptr<imaging::Image>&);

            size_t getAudioBufferSize() const;
            int decodeAudio();
        };

        void Read::_init(
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, options, logSystem);

            TLRENDER_P();

            auto i = options.find("ffmpeg/AudioChannelCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                size_t channelCount = 0;
                ss >> channelCount;
                p.audioConvertInfo.channelCount = std::min(channelCount, static_cast<size_t>(255));
            }
            i = options.find("ffmpeg/AudioDataType");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.audioConvertInfo.dataType;
            }
            i = options.find("ffmpeg/AudioSampleRate");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.audioConvertInfo.sampleRate;
            }
            i = options.find("ffmpeg/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.threadCount;
            }

            p.running = true;
            p.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
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
                        p.infoPromise.set_value(io::Info());
                    }

                    {
                        std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
                        std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            p.stopped = true;
                            videoRequests = std::move(p.videoRequests);
                            audioRequests = std::move(p.audioRequests);
                        }
                        while (!videoRequests.empty())
                        {
                            videoRequests.front()->promise.set_value(io::VideoData());
                            videoRequests.pop_front();
                        }
                        while (!audioRequests.empty())
                        {
                            audioRequests.front()->promise.set_value(io::AudioData());
                            audioRequests.pop_front();
                        }
                    }

                    _close();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, options, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            return _p->infoPromise.get_future();
        }

        std::future<io::VideoData> Read::readVideo(
            const otime::RationalTime& time,
            uint16_t)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        std::future<io::AudioData> Read::readAudio(const otime::TimeRange& time)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        bool Read::hasRequests()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return !p.videoRequests.empty() || !p.audioRequests.empty();
        }

        void Read::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequestsCleanup;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequestsCleanup;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                videoRequestsCleanup = std::move(p.videoRequests);
                audioRequestsCleanup = std::move(p.audioRequests);
            }
            for (auto& request : videoRequestsCleanup)
            {
                request->promise.set_value(io::VideoData());
            }
            for (auto& request : audioRequestsCleanup)
            {
                request->promise.set_value(io::AudioData());
            }
        }

        void Read::stop()
        {
            _p->running = false;
        }

        bool Read::hasStopped() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.stopped;
        }

        void Read::_open(const std::string& fileName)
        {
            TLRENDER_P();

            int r = avformat_open_input(
                &p.video.avFormatContext,
                fileName.c_str(),
                nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            r = avformat_find_stream_info(p.video.avFormatContext, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            for (unsigned int i = 0; i < p.video.avFormatContext->nb_streams; ++i)
            {
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
                r = avcodec_parameters_copy(p.video.avCodecParameters[p.video.avStream], avVideoCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.video.avCodecContext[p.video.avStream] = avcodec_alloc_context3(avVideoCodec);
                r = avcodec_parameters_to_context(p.video.avCodecContext[p.video.avStream], p.video.avCodecParameters[p.video.avStream]);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.video.avCodecContext[p.video.avStream]->thread_count = p.threadCount;
                p.video.avCodecContext[p.video.avStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(p.video.avCodecContext[p.video.avStream], avVideoCodec, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }

                imaging::Info videoInfo;
                videoInfo.size.w = p.video.avCodecParameters[p.video.avStream]->width;
                videoInfo.size.h = p.video.avCodecParameters[p.video.avStream]->height;
                videoInfo.layout.mirror.y = true;

                p.video.avFrame = av_frame_alloc();
                const AVPixelFormat avPixelFormat = static_cast<AVPixelFormat>(
                    p.video.avCodecParameters[p.video.avStream]->format);
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
                {
                    videoInfo.pixelType = imaging::PixelType::YUV_420P;
                    p.video.avFrame2 = av_frame_alloc();
                    p.video.swsContext = sws_getContext(
                        p.video.avCodecParameters[p.video.avStream]->width,
                        p.video.avCodecParameters[p.video.avStream]->height,
                        avPixelFormat,
                        p.video.avCodecParameters[p.video.avStream]->width,
                        p.video.avCodecParameters[p.video.avStream]->height,
                        AV_PIX_FMT_YUV420P,
                        swsScaleFlags,
                        0,
                        0,
                        0);
                    const int srcColorRange = p.video.avCodecContext[p.video.avStream]->color_range;
                    int srcColorSpace = SWS_CS_DEFAULT;
                    if (p.video.avCodecContext[p.video.avStream]->color_primaries != AVCOL_PRI_UNSPECIFIED)
                    {
                        switch (p.video.avCodecContext[p.video.avStream]->colorspace)
                        {
                        case AVCOL_SPC_BT709: srcColorSpace = SWS_CS_ITU709; break;
                        default: break;
                        }
                    }
                    sws_setColorspaceDetails(
                        p.video.swsContext,
                        sws_getCoefficients(srcColorSpace),
                        AVCOL_RANGE_JPEG == srcColorRange ? 1 : 0,
                        sws_getCoefficients(SWS_CS_DEFAULT),
                        1,
                        0,
                        1 << 16,
                        1 << 16);
                    break;
                }
                }
                if (p.video.avCodecContext[p.video.avStream]->color_range != AVCOL_RANGE_JPEG)
                {
                    videoInfo.yuvRange = imaging::YUVRange::Video;
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
                const float speed = avVideoStream->r_frame_rate.num / double(avVideoStream->r_frame_rate.den);
                p.info.videoTime = otime::TimeRange(
                    otime::RationalTime(0.0, speed),
                    otime::RationalTime(
                        sequenceSize,
                        avVideoStream->r_frame_rate.num / double(avVideoStream->r_frame_rate.den)));

                p.videoTime = otime::RationalTime(0.0, speed);

                AVDictionaryEntry* tag = nullptr;
                while ((tag = av_dict_get(p.video.avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                {
-                    const std::string key(tag->key);
-                    const std::string value(tag->value);
-                    tags[key] = value;
-                    if (string::compareNoCase(key, "timecode"))
-                    {
-                        otime::ErrorStatus errorStatus;
-                        const otime::RationalTime time = otime::RationalTime::from_timecode(
-                            value,
-                            speed,
-                            &errorStatus);
-                        if (!otime::is_error(errorStatus))
-                        {
-                            startTime = time::floor(time.rescaled_to(speed));
-                            //std::cout << "start time: " << startTime << std::endl;
-                        }
-                    }
                }

                p.info.videoTime = otime::TimeRange(
                    startTime,
                    otime::RationalTime(sequenceSize, speed));

                p.videoTime = p.info.videoTime.start_time();

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
                    ss << videoInfo.pixelAspectRatio;
                    p.info.tags["Video Pixel Aspect Ratio"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << videoInfo.pixelType;
                    p.info.tags["Video Pixel Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << videoInfo.yuvRange;
                    p.info.tags["Video YUV Range"] = ss.str();
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

            r = avformat_open_input(
                &p.audio.avFormatContext,
                fileName.c_str(),
                nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            r = avformat_find_stream_info(p.audio.avFormatContext, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            for (unsigned int i = 0; i < p.audio.avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_AUDIO == p.audio.avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT == p.audio.avFormatContext->streams[i]->disposition)
                {
                    p.audio.avStream = i;
                    break;
                }
            }
            if (-1 == p.audio.avStream)
            {
                for (unsigned int i = 0; i < p.audio.avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_AUDIO == p.audio.avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        p.audio.avStream = i;
                        break;
                    }
                }
            }
            if (p.audio.avStream != -1)
            {
                //av_dump_format(p.audio.avFormatContext, p.audio.avStream, fileName.c_str(), 0);

                auto avAudioStream = p.audio.avFormatContext->streams[p.audio.avStream];
                auto avAudioCodecParameters = avAudioStream->codecpar;
                auto avAudioCodec = avcodec_find_decoder(avAudioCodecParameters->codec_id);
                if (!avAudioCodec)
                {
                    throw std::runtime_error(string::Format("{0}: No audio codec found").arg(fileName));
                }
                p.audio.avCodecParameters[p.audio.avStream] = avcodec_parameters_alloc();
                r = avcodec_parameters_copy(p.audio.avCodecParameters[p.audio.avStream], avAudioCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.audio.avCodecContext[p.audio.avStream] = avcodec_alloc_context3(avAudioCodec);
                r = avcodec_parameters_to_context(p.audio.avCodecContext[p.audio.avStream], p.audio.avCodecParameters[p.audio.avStream]);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.audio.avCodecContext[p.audio.avStream]->thread_count = p.threadCount;
                p.audio.avCodecContext[p.audio.avStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(p.audio.avCodecContext[p.audio.avStream], avAudioCodec, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }

                size_t channelCount = p.audio.avCodecParameters[p.audio.avStream]->channels;
                switch (channelCount)
                {
                case 1:
                case 2:
                case 6:
                case 7:
                case 8: break;
                default:
                    throw std::runtime_error(string::Format("{0}: Unsupported audio channels").arg(fileName));
                    break;
                }
                audio::DataType dataType = toAudioType(static_cast<AVSampleFormat>(
                    p.audio.avCodecParameters[p.audio.avStream]->format));
                if (audio::DataType::None == dataType)
                {
                    throw std::runtime_error(string::Format("{0}: Unsupported audio format").arg(fileName));
                }
                size_t sampleRate = p.audio.avCodecParameters[p.audio.avStream]->sample_rate;
                if (p.audioConvertInfo.isValid())
                {
                    if ( p.audioConvertInfo.channelCount < channelCount )
                    {
                        channelCount = p.audioConvertInfo.channelCount;
                        dataType = p.audioConvertInfo.dataType;
                        sampleRate = p.audioConvertInfo.sampleRate;
                    }
                }

                p.audio.avFrame = av_frame_alloc();

                int64_t sampleCount = 0;
                if (avAudioStream->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = 1;
                    r.den = sampleRate;
                    sampleCount = av_rescale_q(
                        avAudioStream->duration,
                        avAudioStream->time_base,
                        r);
                }
                else if (p.audio.avFormatContext->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = 1;
                    r.den = sampleRate;
                    sampleCount = av_rescale_q(
                        p.audio.avFormatContext->duration,
                        av_get_time_base_q(),
                        r);
                }

                p.info.audio.channelCount = channelCount;
                p.info.audio.dataType = dataType;
                p.info.audio.sampleRate = sampleRate;
                p.info.audioTime = otime::TimeRange::range_from_start_end_time(
                    otime::RationalTime(0.0, sampleRate),
                    otime::RationalTime(sampleCount, sampleRate));

                p.audioTime = otime::RationalTime(0.0, sampleRate);

                AVDictionaryEntry* tag = nullptr;
                while ((tag = av_dict_get(p.audio.avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                {
                    p.info.tags[tag->key] = tag->value;
                }
            }

            p.infoPromise.set_value(p.info);
        }

        void Read::_run()
        {
            TLRENDER_P();

            if (p.video.avStream != -1)
            {
                p.video.avFrame = av_frame_alloc();
                const AVPixelFormat avPixelFormat = static_cast<AVPixelFormat>(
                    p.video.avCodecParameters[p.video.avStream]->format);
                switch (avPixelFormat)
                {
                case AV_PIX_FMT_YUV420P:
                case AV_PIX_FMT_RGB24:
                case AV_PIX_FMT_GRAY8:
                case AV_PIX_FMT_RGBA: break;
                default:
                {
                    p.video.avFrame2 = av_frame_alloc();
                    p.video.swsContext = sws_getContext(
                        p.video.avCodecParameters[p.video.avStream]->width,
                        p.video.avCodecParameters[p.video.avStream]->height,
                        avPixelFormat,
                        p.video.avCodecParameters[p.video.avStream]->width,
                        p.video.avCodecParameters[p.video.avStream]->height,
                        AV_PIX_FMT_YUV420P,
                        swsScaleFlags,
                        0,
                        0,
                        0);
                    const int srcColorRange = p.video.avCodecContext[p.video.avStream]->color_range;
                    int srcColorSpace = SWS_CS_DEFAULT;
                    if (p.video.avCodecContext[p.video.avStream]->color_primaries != AVCOL_PRI_UNSPECIFIED)
                    {
                        switch (p.video.avCodecContext[p.video.avStream]->colorspace)
                        {
                        case AVCOL_SPC_BT709: srcColorSpace = SWS_CS_ITU709; break;
                        default: break;
                        }
                    }
                    sws_setColorspaceDetails(
                        p.video.swsContext,
                        sws_getCoefficients(srcColorSpace),
                        AVCOL_RANGE_JPEG == srcColorRange ? 1 : 0,
                        sws_getCoefficients(SWS_CS_DEFAULT),
                        1,
                        0,
                        1 << 16,
                        1 << 16);
                    break;
                }
                }
            }

            if (p.audio.avStream != -1)
            {
                uint64_t channelLayout = p.audio.avCodecParameters[p.audio.avStream]->channel_layout;
                if (0 == channelLayout)
                {
                    std::bitset<64> bs;
                    for (size_t i = 0; i < p.audio.avCodecParameters[p.audio.avStream]->channels; ++i)
                    {
                        bs[i] = 1;
                    }
                    channelLayout = bs.to_ulong();
                }
                p.audio.swrContext = swr_alloc_set_opts(
                    NULL,
                    fromChannelCount(p.info.audio.channelCount),
                    fromAudioType(p.info.audio.dataType),
                    p.info.audio.sampleRate,
                    channelLayout,
                    static_cast<AVSampleFormat>(p.audio.avCodecParameters[p.audio.avStream]->format),
                    p.audio.avCodecParameters[p.audio.avStream]->sample_rate,
                    0,
                    NULL);
                swr_init(p.audio.swrContext);
            }

            p.logTimer = std::chrono::steady_clock::now();
            while (p.running)
            {
                std::shared_ptr<Private::VideoRequest> videoRequest;
                std::shared_ptr<Private::AudioRequest> audioRequest;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.requestCV.wait_for(
                        lock,
                        requestTimeout,
                        [this]
                        {
                            return !_p->videoRequests.empty() || !_p->audioRequests.empty();
                        }))
                    {
                        if (!p.videoRequests.empty())
                        {
                            videoRequest = p.videoRequests.front();
                            p.videoRequests.pop_front();
                        }
                        if (!p.audioRequests.empty())
                        {
                            audioRequest = p.audioRequests.front();
                            p.audioRequests.pop_front();
                        }
                    }
                }

                if (videoRequest)
                {
                    //std::cout << "video request: " << videoRequest->time << std::endl;
                    if (videoRequest->time != p.videoTime)
                    {
                        //std::cout << "video seek: " << videoRequest->time << std::endl;
                        p.videoTime = videoRequest->time;
                        if (p.video.avStream != -1)
                        {
                            avcodec_flush_buffers(p.video.avCodecContext[p.video.avStream]);
                            if (av_seek_frame(
                                p.video.avFormatContext,
                                p.video.avStream,
                                av_rescale_q(
                                    videoRequest->time.value(),
                                    swap(p.video.avFormatContext->streams[p.video.avStream]->r_frame_rate),
                                    p.video.avFormatContext->streams[p.video.avStream]->time_base),
                                AVSEEK_FLAG_BACKWARD) < 0)
                            {
                                //! \todo How should this be handled?
                            }

                            p.video.buffer.clear();
                        }
                    }

                    AVPacket packet;
                    av_init_packet(&packet);
                    int decoding = 0;
                    bool eof = false;
                    while (0 == decoding)
                    {
                        if (!eof)
                        {
                            decoding = av_read_frame(p.video.avFormatContext, &packet);
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
                        if ((eof && p.video.avStream != -1) || (p.video.avStream == packet.stream_index))
                        {
                            decoding = avcodec_send_packet(
                                p.video.avCodecContext[p.video.avStream],
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
                            decoding = p.decodeVideo();
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

                    io::VideoData data;
                    data.time = videoRequest->time;
                    if (!p.video.buffer.empty())
                    {
                        data.image = p.video.buffer.front();
                        p.video.buffer.pop_front();
                    }
                    videoRequest->promise.set_value(data);

                    p.videoTime += otime::RationalTime(1.0, p.info.videoTime.duration().rate());
                }

                if (audioRequest)
                {
                    //std::cout << "audio request: " << audioRequest->time << std::endl;
                    if (audioRequest->time.start_time() != p.audioTime)
                    {
                        //std::cout << "audio seek: " << audioRequest->time << std::endl;
                        p.audioTime = audioRequest->time.start_time();
                        if (p.audio.avStream != -1)
                        {
                            avcodec_flush_buffers(p.audio.avCodecContext[p.audio.avStream]);
                            AVRational r;
                            r.num = 1;
                            r.den = p.info.audio.sampleRate;
                            if (av_seek_frame(
                                p.audio.avFormatContext,
                                p.audio.avStream,
                                av_rescale_q(
                                    audioRequest->time.start_time().value(),
                                    r,
                                    p.audio.avFormatContext->streams[p.audio.avStream]->time_base),
                                AVSEEK_FLAG_BACKWARD) < 0)
                            {
                                //! \todo How should this be handled?
                            }

                            std::vector<uint8_t> swrOutputBuffer;
                            swrOutputBuffer.resize(
                                static_cast<size_t>(p.info.audio.channelCount) *
                                audio::getByteCount(p.info.audio.dataType) *
                                p.audio.avFrame->nb_samples);
                            uint8_t* swrOutputBufferP[] = { swrOutputBuffer.data() };
                            while (swr_convert(
                                p.audio.swrContext,
                                swrOutputBufferP,
                                p.audio.avFrame->nb_samples,
                                NULL,
                                0) > 0)
                                ;
                            swr_init(p.audio.swrContext);

                            p.audio.buffer.clear();
                        }
                    }

                    AVPacket packet;
                    av_init_packet(&packet);
                    int decoding = 0;
                    bool eof = false;
                    while (0 == decoding && p.getAudioBufferSize() < audioRequest->time.duration().value())
                    {
                        if (!eof)
                        {
                            decoding = av_read_frame(p.audio.avFormatContext, &packet);
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
                        if ((eof && p.audio.avStream != -1) || (p.audio.avStream == packet.stream_index))
                        {
                            decoding = avcodec_send_packet(
                                p.audio.avCodecContext[p.audio.avStream],
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
                            decoding = p.decodeAudio();
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
                                decoding = 0;
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

                    io::AudioData data;
                    data.time = audioRequest->time.start_time();
                    data.audio = audio::Audio::create(p.info.audio, audioRequest->time.duration().value());
                    data.audio->zero();
                    audio::copy(p.audio.buffer, data.audio->getData(), data.audio->getByteCount());
                    audioRequest->promise.set_value(data);

                    p.audioTime += audioRequest->time.duration();
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.logTimer = now;
                        const std::string id = string::Format("tl::io::ffmpeg::Read {0}").arg(this);
                        size_t videoRequestsSize = 0;
                        size_t audioRequestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            videoRequestsSize = p.videoRequests.size();
                            audioRequestsSize = p.audioRequests.size();
                        }
                        logSystem->print(id, string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Video requests: {1}\n"
                            "    Audio requests: {2}\n"
                            "    Thread count: {3}").
                            arg(_path.get()).
                            arg(videoRequestsSize).
                            arg(audioRequestsSize).
                            arg(p.threadCount));
                    }
                }
            }
        }

        void Read::_close()
        {
            TLRENDER_P();

            if (p.video.swsContext)
            {
                sws_freeContext(p.video.swsContext);
            }
            if (p.video.avFrame2)
            {
                av_frame_free(&p.video.avFrame2);
            }
            if (p.video.avFrame)
            {
                av_frame_free(&p.video.avFrame);
            }
            for (auto i : p.video.avCodecContext)
            {
                avcodec_close(i.second);
                avcodec_free_context(&i.second);
            }
            for (auto i : p.video.avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (p.video.avFormatContext)
            {
                avformat_close_input(&p.video.avFormatContext);
            }

            if (p.audio.swrContext)
            {
                swr_free(&p.audio.swrContext);
            }
            if (p.audio.avFrame)
            {
                av_frame_free(&p.audio.avFrame);
            }
            for (auto i : p.audio.avCodecContext)
            {
                avcodec_close(i.second);
                avcodec_free_context(&i.second);
            }
            for (auto i : p.audio.avCodecParameters)
            {
                avcodec_parameters_free(&i.second);
            }
            if (p.audio.avFormatContext)
            {
                avformat_close_input(&p.audio.avFormatContext);
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
                    av_rescale_q(
                        timestamp,
                        video.avFormatContext->streams[video.avStream]->time_base,
                        swap(video.avFormatContext->streams[video.avStream]->r_frame_rate)),
                    info.videoTime.duration().rate());
                //std::cout << "video time: " << time << std::endl;

                if (time >= videoTime)
                {
                    //std::cout << "video frame: " << time << std::endl;
                    auto image = imaging::Image::create(info.video[0]);
                    image->setTags(info.tags);
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
            default:
                av_image_fill_arrays(
                    video.avFrame2->data,
                    video.avFrame2->linesize,
                    data,
                    AV_PIX_FMT_YUV420P,
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

        size_t Read::Private::getAudioBufferSize() const
        {
            size_t out = 0;
            for (const auto& i : audio.buffer)
            {
                out += i->getSampleCount();
            }
            return out;
        }

        int Read::Private::decodeAudio()
        {
            int out = 0;
            while (0 == out)
            {
                out = avcodec_receive_frame(audio.avCodecContext[audio.avStream], audio.avFrame);
                if (out < 0)
                {
                    return out;
                }
                const int64_t timestamp = audio.avFrame->pts != AV_NOPTS_VALUE ? audio.avFrame->pts : audio.avFrame->pkt_dts;
                //std::cout << "audio timestamp: " << timestamp << std::endl;

                AVRational r;
                r.num = 1;
                r.den = info.audio.sampleRate;
                const auto time = otime::RationalTime(
                    av_rescale_q(
                        timestamp,
                        audio.avFormatContext->streams[audio.avStream]->time_base,
                        r),
                    info.audio.sampleRate);
                //std::cout << "audio time: " << time << std::endl;

                if (time >= audioTime)
                {
                    //std::cout << "audio time: " << time << std::endl;
                    const int64_t swrDelay = swr_get_delay(audio.swrContext, audio.avCodecParameters[audio.avStream]->sample_rate);
                    //std::cout << "delay: " << swrDelay << std::endl;
                    const size_t swrOutputSamples = audio.avFrame->nb_samples + swrDelay;
                    std::vector<uint8_t> swrOutputBuffer;
                    swrOutputBuffer.resize(
                        static_cast<size_t>(info.audio.channelCount) *
                        audio::getByteCount(info.audio.dataType) *
                        swrOutputSamples);
                    uint8_t* swrOutputBufferP[] = { swrOutputBuffer.data() };
                    const int swrOutputCount = swr_convert(
                        audio.swrContext,
                        swrOutputBufferP,
                        swrOutputSamples,
                        (const uint8_t **)audio.avFrame->data,
                        audio.avFrame->nb_samples);
                    if (swrOutputCount < 0)
                    {
                        //! \todo How should this be handled?
                        break;
                    }
                    auto tmp = audio::Audio::create(info.audio, swrOutputCount);
                    memcpy(tmp->getData(), swrOutputBuffer.data(), tmp->getByteCount());
                    audio.buffer.push_back(tmp);
                    out = 1;
                    break;
                }
            }
            return out;
        }
    }
}
