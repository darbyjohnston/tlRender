// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/LogSystem.h>

extern "C"
{
#include <libavutil/opt.h>

} // extern "C"

namespace tl
{
    namespace ffmpeg
    {
        AVIOBufferData::AVIOBufferData()
        {}

        AVIOBufferData::AVIOBufferData(const uint8_t* p, size_t size) :
            p(p),
            size(size)
        {}

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            const int64_t remaining = bufferData->size - bufferData->offset;
            int bufSizeClamped = ftk::clamp(
                static_cast<int64_t>(bufSize),
                static_cast<int64_t>(0),
                remaining);
            if (!bufSizeClamped)
            {
                return AVERROR_EOF;
            }

            memcpy(buf, bufferData->p + bufferData->offset, bufSizeClamped);
            bufferData->offset += bufSizeClamped;

            return bufSizeClamped;
        }

        int64_t avIOBufferSeek(void* opaque, int64_t offset, int whence)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            if (whence & AVSEEK_SIZE)
            {
                return bufferData->size;
            }

            bufferData->offset = ftk::clamp(
                offset,
                static_cast<int64_t>(0),
                static_cast<int64_t>(bufferData->size));

            return offset;
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);
            FTK_P();

            auto i = options.find("FFmpeg/YUVToRGB");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.yuvToRGBConversion;
            }
            i = options.find("FFmpeg/AudioChannelCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.channelCount;
            }
            i = options.find("FFmpeg/AudioDataType");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.dataType;
            }
            i = options.find("FFmpeg/AudioSampleRate");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.sampleRate;
            }
            i = options.find("FFmpeg/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.threadCount;
            }
            i = options.find("FFmpeg/RequestTimeout");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.requestTimeout;
            }
            i = options.find("FFmpeg/VideoBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.videoBufferSize;
            }
            i = options.find("FFmpeg/AudioBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioBufferSize;
            }

            p.videoThread.running = true;
            p.audioThread.running = true;
            p.videoThread.thread = std::thread(
                [this, path]
                {
                    FTK_P();
                    try
                    {
                        p.readVideo = std::make_shared<ReadVideo>(
                            path.get(-1, path.isFileProtocol() ? file::PathType::Path : file::PathType::Full),
                            _memory,
                            p.options);
                        const auto& videoInfo = p.readVideo->getInfo();
                        if (videoInfo.isValid())
                        {
                            p.info.video.push_back(videoInfo);
                            p.info.videoTime = p.readVideo->getTimeRange();
                            p.info.tags = p.readVideo->getTags();
                        }

                        p.readAudio = std::make_shared<ReadAudio>(
                            path.get(-1, path.isFileProtocol() ? file::PathType::Path : file::PathType::Full),
                            _memory,
                            p.info.videoTime.duration().rate(),
                            p.options);
                        p.info.audio = p.readAudio->getInfo();
                        p.info.audioTime = p.readAudio->getTimeRange();
                        for (const auto& tag : p.readAudio->getTags())
                        {
                            p.info.tags[tag.first] = tag.second;
                        }

                        p.audioThread.thread = std::thread(
                            [this, path]
                            {
                                FTK_P();
                                try
                                {
                                    _audioThread();
                                }
                                catch (const std::exception& e)
                                {
                                    if (auto logSystem = _logSystem.lock())
                                    {
                                        //! \todo How should this be handled?
                                        logSystem->print(
                                            "tl::io::ffmpeg::Read",
                                            e.what(),
                                            ftk::LogType::Error);
                                    }
                                }
                            });

                        _videoThread();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Read",
                                e.what(),
                                ftk::LogType::Error);
                        }
                    }

                    {
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        p.videoMutex.stopped = true;
                    }
                    _cancelVideoRequests();
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.stopped = true;
                    }
                    _cancelAudioRequests();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            FTK_P();
            p.videoThread.running = false;
            p.audioThread.running = false;
            if (p.videoThread.thread.joinable())
            {
                p.videoThread.thread.join();
            }
            if (p.audioThread.thread.joinable())
            {
                p.audioThread.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<ftk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            FTK_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.stopped)
                {
                    valid = true;
                    p.videoMutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.videoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return future;
        }

        std::future<io::VideoData> Read::readVideo(
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->options = io::merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.stopped)
                {
                    valid = true;
                    p.videoMutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.videoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        std::future<io::AudioData> Read::readAudio(
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            FTK_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->timeRange = timeRange;
            request->options = io::merge(options, _options);
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (!p.audioMutex.stopped)
                {
                    valid = true;
                    p.audioMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.audioThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        void Read::cancelRequests()
        {
            _cancelVideoRequests();
            _cancelAudioRequests();
        }

        void Read::_videoThread()
        {
            FTK_P();
            p.videoThread.currentTime = p.info.videoTime.start_time();
            p.readVideo->start();
            p.videoThread.logTimer = std::chrono::steady_clock::now();
            while (p.videoThread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::shared_ptr<Private::VideoRequest> videoRequest;
                {
                    std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                    if (p.videoThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(p.options.requestTimeout),
                        [this]
                        {
                            return
                                !_p->videoMutex.infoRequests.empty() ||
                                !_p->videoMutex.videoRequests.empty();
                        }))
                    {
                        infoRequests = std::move(p.videoMutex.infoRequests);
                        if (!p.videoMutex.videoRequests.empty())
                        {
                            videoRequest = p.videoMutex.videoRequests.front();
                            p.videoMutex.videoRequests.pop_front();
                        }
                    }
                }

                // Information requests.
                for (auto& request : infoRequests)
                {
                    request->promise.set_value(p.info);
                }

                // Seek.
                if (videoRequest &&
                    !videoRequest->time.strictly_equal(p.videoThread.currentTime))
                {
                    p.videoThread.currentTime = videoRequest->time;
                    p.readVideo->seek(p.videoThread.currentTime);
                }

                // Process.
                while (
                    videoRequest &&
                    p.readVideo->isBufferEmpty() &&
                    p.readVideo->isValid() &&
                    _p->readVideo->process(p.videoThread.currentTime))
                    ;

                // Handle request.
                if (videoRequest)
                {
                    io::VideoData data;
                    data.time = videoRequest->time;
                    if (!p.readVideo->isBufferEmpty())
                    {
                        data.image = p.readVideo->popBuffer();
                    }
                    videoRequest->promise.set_value(data);
                    
                    p.videoThread.currentTime += OTIO_NS::RationalTime(1.0, p.info.videoTime.duration().rate());
                }

                // Logging.
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.videoThread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.videoThread.logTimer = now;
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = ftk::Format("tl::io::ffmpeg::Read {0}").arg(this);
                            size_t requestsSize = 0;
                            {
                                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                                requestsSize = p.videoMutex.videoRequests.size();
                            }
                            logSystem->print(id, ftk::Format(
                                "\n"
                                "    Path: {0}\n"
                                "    Video requests: {1}").
                                arg(_path.get()).
                                arg(requestsSize));
                        }
                    }
                }
            }
        }

        void Read::_audioThread()
        {
            FTK_P();
            p.audioThread.currentTime = p.info.audioTime.start_time();
            p.readAudio->start();
            p.audioThread.logTimer = std::chrono::steady_clock::now();
            while (p.audioThread.running)
            {
                // Check requests.
                std::shared_ptr<Private::AudioRequest> request;
                size_t requestSampleCount = 0;
                bool seek = false;
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    if (p.audioThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(p.options.requestTimeout),
                        [this]
                        {
                            return !_p->audioMutex.requests.empty();
                        }))
                    {
                        if (!p.audioMutex.requests.empty())
                        {
                            request = p.audioMutex.requests.front();
                            p.audioMutex.requests.pop_front();
                            requestSampleCount = request->timeRange.duration().rescaled_to(p.info.audio.sampleRate).value();
                            if (!request->timeRange.start_time().strictly_equal(p.audioThread.currentTime))
                            {
                                seek = true;
                                p.audioThread.currentTime = request->timeRange.start_time();
                            }
                        }
                    }
                }

                // Seek.
                if (seek)
                {
                    p.readAudio->seek(p.audioThread.currentTime);
                }

                // Process.
                bool intersects = false;
                if (request)
                {
                    intersects = request->timeRange.intersects(p.info.audioTime);
                }
                while (
                    request &&
                    intersects &&
                    p.readAudio->getBufferSize() < request->timeRange.duration().rescaled_to(p.info.audio.sampleRate).value() &&
                    p.readAudio->isValid() &&
                    p.readAudio->process(
                        p.audioThread.currentTime,
                        requestSampleCount ?
                        requestSampleCount :
                        p.options.audioBufferSize.rescaled_to(p.info.audio.sampleRate).value()))
                    ;

                // Handle request.
                if (request)
                {
                    io::AudioData audioData;
                    audioData.time = request->timeRange.start_time();
                    audioData.audio = audio::Audio::create(p.info.audio, request->timeRange.duration().value());
                    audioData.audio->zero();
                    if (intersects)
                    {
                        size_t offset = 0;
                        if (audioData.time < p.info.audioTime.start_time())
                        {
                            offset = (p.info.audioTime.start_time() - audioData.time).value();
                        }
                        p.readAudio->bufferCopy(
                            audioData.audio->getData() + offset * p.info.audio.getByteCount(),
                            audioData.audio->getSampleCount() - offset);
                    }
                    request->promise.set_value(audioData);

                    p.audioThread.currentTime += request->timeRange.duration();
                }

                // Logging.
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.audioThread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.audioThread.logTimer = now;
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = ftk::Format("tl::io::ffmpeg::Read {0}").arg(this);
                            size_t requestsSize = 0;
                            {
                                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                                requestsSize = p.audioMutex.requests.size();
                            }
                            logSystem->print(id, ftk::Format(
                                "\n"
                                "    Path: {0}\n"
                                "    Audio requests: {1}").
                                arg(_path.get()).
                                arg(requestsSize));
                        }
                    }
                }
            }
        }

        void Read::_cancelVideoRequests()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                infoRequests = std::move(p.videoMutex.infoRequests);
                videoRequests = std::move(p.videoMutex.videoRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(io::VideoData());
            }
        }

        void Read::_cancelAudioRequests()
        {
            FTK_P();
            std::list<std::shared_ptr<Private::AudioRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                requests = std::move(p.audioMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::AudioData());
            }
        }
    }
}
