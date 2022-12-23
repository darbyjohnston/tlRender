// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

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
            pCurrent(p),
            size(size)
        {}

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            const size_t remaining = (bufferData->p + bufferData->size) - bufferData->pCurrent;
            bufSize = std::min(static_cast<size_t>(bufSize), remaining);
            if (!bufSize)
            {
                return AVERROR_EOF;
            }

            memcpy(buf, bufferData->pCurrent, bufSize);
            bufferData->pCurrent += bufSize;

            return bufSize;
        }

        int64_t avIOBufferSeek(void* opaque, int64_t offset, int whence)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            if (whence & AVSEEK_SIZE)
            {
                return bufferData->size;
            }

            bufferData->pCurrent = bufferData->p + offset;

            return offset;
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);

            TLRENDER_P();

            auto i = options.find("ffmpeg/YUVToRGBConversion");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.yuvToRGBConversion;
            }
            i = options.find("ffmpeg/AudioChannelCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                size_t channelCount = 0;
                ss >> channelCount;
                p.options.audioConvertInfo.channelCount = std::min(channelCount, static_cast<size_t>(255));
            }
            i = options.find("ffmpeg/AudioDataType");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.dataType;
            }
            i = options.find("ffmpeg/AudioSampleRate");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.sampleRate;
            }
            i = options.find("ffmpeg/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.threadCount;
            }
            i = options.find("ffmpeg/RequestTimeout");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.requestTimeout;
            }
            i = options.find("ffmpeg/VideoBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.videoBufferSize;
            }
            i = options.find("ffmpeg/AudioBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioBufferSize;
            }

            p.running = true;
            p.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.readVideo = std::make_shared<ReadVideo>(path.get(), _memory, p.options);
                        p.info.video.push_back(p.readVideo->getInfo());
                        p.info.videoTime = p.readVideo->getTimeRange();
                        p.info.tags = p.readVideo->getTags();

                        p.readAudio = std::make_shared<ReadAudio>(path.get(), _memory, p.info.videoTime.duration().rate(), p.options);
                        p.info.audio = p.readAudio->getInfo();
                        p.info.audioTime = p.readAudio->getTimeRange();
                        for (const auto& tag : p.readAudio->getTags())
                        {
                            p.info.tags[tag.first] = tag.second;
                        }

                        p.infoPromise.set_value(p.info);

                        try
                        {
                            _run();
                        }
                        catch (const std::exception& e)
                        {
                            if (auto logSystem = _logSystem.lock())
                            {
                                const std::string id = string::Format("tl::io::ffmpeg::Read ({0}: {1})").
                                    arg(__FILE__).
                                    arg(__LINE__);
                                logSystem->print(id, string::Format("{0}: {1}").
                                    arg(_path.get()).
                                    arg(e.what()),
                                    log::Type::Error);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = string::Format("tl::io::ffmpeg::Read ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, string::Format("{0}: {1}").
                                arg(_path.get()).
                                arg(e.what()),
                                log::Type::Error);
                        }
                        p.infoPromise.set_value(io::Info());
                    }

                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.stopped = true;
                    }
                    cancelRequests();
                    p.readVideo.reset();
                    p.readAudio.reset();
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
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
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
                p.cv.notify_one();
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
                p.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        void Read::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                videoRequests = std::move(p.videoRequests);
                if (p.currentVideoRequest)
                {
                    videoRequests.push_front(p.currentVideoRequest);
                    p.currentVideoRequest.reset();
                }
                audioRequests = std::move(p.audioRequests);
                if (p.currentAudioRequest)
                {
                    audioRequests.push_front(p.currentAudioRequest);
                    p.currentAudioRequest.reset();
                }
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(io::VideoData());
            }
            for (auto& request : audioRequests)
            {
                request->promise.set_value(io::AudioData());
            }
        }

        void Read::_run()
        {
            TLRENDER_P();

            p.currentVideoTime = p.info.videoTime.start_time();
            p.currentAudioTime = p.info.audioTime.start_time();

            p.readVideo->start();
            p.readAudio->start();

            p.logTimer = std::chrono::steady_clock::now();
            while (p.running)
            {
                // Check requests.
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(p.options.requestTimeout),
                        [this]
                        {
                            return
                                !_p->videoRequests.empty() ||
                                _p->currentVideoRequest ||
                                !_p->audioRequests.empty() ||
                                _p->currentAudioRequest ||
                                !_p->readVideo->isBufferFull() ||
                                !_p->readAudio->isBufferFull();
                        }))
                    {
                        if (!p.currentVideoRequest && !p.videoRequests.empty())
                        {
                            p.currentVideoRequest = p.videoRequests.front();
                            p.videoRequests.pop_front();
                        }
                        if (!p.currentAudioRequest && !p.audioRequests.empty())
                        {
                            p.currentAudioRequest = p.audioRequests.front();
                            p.audioRequests.pop_front();
                        }
                    }
                }

                // Seek.
                {
                    bool seek = false;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if (p.currentVideoRequest)
                        {
                            if (!time::compareExact(p.currentVideoRequest->time, p.currentVideoTime))
                            {
                                seek = true;
                                p.currentVideoTime = p.currentVideoRequest->time;
                            }
                        }
                    }
                    if (seek)
                    {
                        p.readVideo->seek(p.currentVideoTime);
                    }
                }
                {
                    bool seek = false;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if (p.currentAudioRequest)
                        {
                            if (!time::compareExact(p.currentAudioRequest->time.start_time(), p.currentAudioTime))
                            {
                                seek = true;
                                p.currentAudioTime = p.currentAudioRequest->time.start_time();
                            }
                        }
                    }
                    if (seek)
                    {
                        p.readAudio->seek(p.currentAudioTime);
                    }
                }

                // Process.
                _p->readVideo->process(p.currentVideoTime);
                _p->readAudio->process(p.currentAudioTime);

                // Handle requests.
                {
                    std::shared_ptr<Private::VideoRequest> request;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if ((p.currentVideoRequest && !p.readVideo->isBufferEmpty()) ||
                            (p.currentVideoRequest && !p.readVideo->isValid()))
                        {
                            request = std::move(p.currentVideoRequest);
                        }
                    }
                    if (request)
                    {
                        io::VideoData data;
                        data.time = request->time;
                        if (!p.readVideo->isBufferEmpty())
                        {
                            data.image = p.readVideo->popBuffer();
                        }
                        request->promise.set_value(data);

                        p.currentVideoTime += otime::RationalTime(1.0, p.info.videoTime.duration().rate());
                    }
                }
                {
                    const size_t bufferSize = p.readAudio->getBufferSize();
                    std::shared_ptr<Private::AudioRequest> request;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        if ((p.currentAudioRequest &&
                            p.currentAudioRequest->time.duration().rescaled_to(p.info.audio.sampleRate).value() <= bufferSize) ||
                            (p.currentAudioRequest && !p.readAudio->isValid()))
                        {
                            request = std::move(p.currentAudioRequest);
                        }
                    }
                    if (request)
                    {
                        io::AudioData data;
                        data.time = request->time.start_time();
                        data.audio = audio::Audio::create(p.info.audio, request->time.duration().value());
                        data.audio->zero();
                        size_t offset = 0;
                        if (data.time < p.info.audioTime.start_time())
                        {
                            offset = (p.info.audioTime.start_time() - data.time).value() * p.info.audio.getByteCount();
                        }
                        p.readAudio->bufferCopy(data.audio->getData() + offset, data.audio->getByteCount() - offset);
                        request->promise.set_value(data);

                        p.currentAudioTime += request->time.duration();
                    }
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
                            arg(p.options.threadCount));
                    }
                }
            }
        }
    }
}
