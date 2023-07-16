// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IOManager.h>

#include <tlIO/IOSystem.h>

#include <tlCore/LRUCache.h>
#include <tlCore/StringFormat.h>

#include <sstream>

namespace tl
{
    namespace timelineui
    {
        struct IOManager::Private
        {
            std::weak_ptr<system::Context> context;
            io::Options ioOptions;
            std::shared_ptr<observer::Value<bool> > cancelRequests;
            
            struct InfoRequest
            {
                file::Path path;
                std::vector<file::MemoryRead> memoryRead;
                std::promise<io::Info> promise;
            };

            struct VideoRequest
            {
                file::Path path;
                otime::RationalTime time = time::invalidTime;
                std::promise<io::VideoData> promise;
            };

            struct AudioRequest
            {
                file::Path path;
                otime::TimeRange range = time::invalidTimeRange;
                std::promise<io::AudioData> promise;
            };
            
            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::list<std::shared_ptr<AudioRequest> > audioRequests;
                bool cancelRequests = true;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;
            
            struct Thread
            {
                memory::LRUCache<std::string, std::shared_ptr<io::IRead> > cache;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void IOManager::_init(
            const io::Options& ioOptions,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.ioOptions = ioOptions;
            {
                std::stringstream ss;
                ss << 1;
                p.ioOptions["ffmpeg/VideoBufferSize"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << otime::RationalTime(1.0, 1.0);
                p.ioOptions["ffmpeg/AudioBufferSize"] = ss.str();
            }
            p.cancelRequests = observer::Value<bool>::create(false);
            
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    _run();
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    _cancelRequests();
                });
        }

        IOManager::IOManager() :
            _p(new Private)
        {}

        IOManager::~IOManager()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<IOManager> IOManager::create(
            const io::Options& options,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<IOManager>(new IOManager);
            out->_init(options, context);
            return out;
        }

        std::future<io::Info> IOManager::getInfo(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& startTime)
        {
            /*TLRENDER_P();
            std::future<io::Info> out;
            std::shared_ptr<io::IRead> read;
            const std::string fileName = path.get();
            if (!p.cache.get(fileName, read))
            {
                if (auto context = p.context.lock())
                {
                    auto ioSystem = context->getSystem<io::System>();
                    io::Options options = p.ioOptions;
                    options["FFmpeg/StartTime"] = string::Format("{0}").arg(startTime);
                    read = ioSystem->read(path, memoryRead, options);
                    p.cache.add(fileName, read);
                }
            }
            if (read)
            {
                out = read->getInfo();
            }
            else
            {
                std::promise<io::Info> promise;
                out = promise.get_future();
                promise.set_value(io::Info());
            }
            return out;*/
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            request->path = path;
            request->memoryRead = memoryRead;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return future;
        }

        std::future<io::VideoData> IOManager::readVideo(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            /*TLRENDER_P();
            std::future<io::VideoData> out;
            std::shared_ptr<io::IRead> read;
            const std::string fileName = path.get();
            if (!p.cache.get(fileName, read))
            {
                if (auto context = p.context.lock())
                {
                    auto ioSystem = context->getSystem<io::System>();
                    read = ioSystem->read(path, memoryRead, p.ioOptions);
                    p.cache.add(fileName, read);
                }
            }
            if (read)
            {
                out = read->readVideo(time, layer);
            }
            else
            {
                std::promise<io::VideoData> promise;
                out = promise.get_future();
                promise.set_value(io::VideoData());
            }
            return out;*/
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->path = path;
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        std::future<io::AudioData> IOManager::readAudio(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memoryRead,
            const otime::TimeRange& range)
        {
            /*TLRENDER_P();
            std::future<io::AudioData> out;
            std::shared_ptr<io::IRead> read;
            const std::string fileName = path.get();
            if (!p.cache.get(fileName, read))
            {
                if (auto context = p.context.lock())
                {
                    auto ioSystem = context->getSystem<io::System>();
                    read = ioSystem->read(path, memoryRead, p.ioOptions);
                    p.cache.add(fileName, read);
                }
            }
            if (read)
            {
                out = read->readAudio(range);
            }
            else
            {
                std::promise<io::AudioData> promise;
                out = promise.get_future();
                promise.set_value(io::AudioData());
            }
            return out;*/
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->path = path;
            request->range = range;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        void IOManager::cancelRequests()
        {
            TLRENDER_P();
            p.cancelRequests->setAlways(true);
            _cancelRequests();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            p.mutex.cancelRequests = true;
        }

        std::shared_ptr<observer::IValue<bool> > IOManager::observeCancelRequests() const
        {
            return _p->cancelRequests;
        }
        
        void IOManager::_run()
        {
            TLRENDER_P();
            while (p.thread.running)
            {
                // Check requests.
                std::shared_ptr<Private::InfoRequest> infoRequest;
                std::shared_ptr<Private::VideoRequest> videoRequest;
                std::shared_ptr<Private::AudioRequest> audioRequest;
                bool cancelRequests = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(5),
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.videoRequests.empty() ||
                                !_p->mutex.audioRequests.empty() ||
                                _p->mutex.cancelRequests;
                        }))
                    {
                        if (!p.mutex.infoRequests.empty())
                        {
                            infoRequest = p.mutex.infoRequests.front();
                            p.mutex.infoRequests.pop_front();
                        }
                        if (!p.mutex.videoRequests.empty())
                        {
                            videoRequest = p.mutex.videoRequests.front();
                            p.mutex.videoRequests.pop_front();
                        }
                        if (!p.mutex.audioRequests.empty())
                        {
                            audioRequest = p.mutex.audioRequests.front();
                            p.mutex.audioRequests.pop_front();
                        }
                        if (p.mutex.cancelRequests)
                        {
                            cancelRequests = true;
                            p.mutex.cancelRequests = false;
                        }
                    }
                }
                
                // Cancel requests.
                if (cancelRequests)
                {
                    for (const auto& i : p.thread.cache.getValues())
                    {
                        if (i)
                        {
                            i->cancelRequests();
                        }
                    }
                }
                
                // Handle information requests.
                if (infoRequest)
                {
                    io::Info info;
                    std::shared_ptr<io::IRead> read;
                    const std::string& fileName = infoRequest->path.get();
                    if (!p.thread.cache.get(fileName, read))
                    {
                        if (auto context = p.context.lock())
                        {
                            auto ioSystem = context->getSystem<io::System>();
                            read = ioSystem->read(
                                infoRequest->path,
                                infoRequest->memoryRead,
                                p.ioOptions);
                            p.thread.cache.add(fileName, read);
                        }
                    }
                    if (read)
                    {
                        info = read->getInfo().get();
                    }
                    infoRequest->promise.set_value(info);
                }
                
                // Handle video requests.
                if (videoRequest)
                {
                    io::VideoData videoData;
                    std::shared_ptr<io::IRead> read;
                    const std::string& fileName = videoRequest->path.get();
                    if (!p.thread.cache.get(fileName, read))
                    {
                        if (auto context = p.context.lock())
                        {
                            auto ioSystem = context->getSystem<io::System>();
                            read = ioSystem->read(
                                infoRequest->path,
                                infoRequest->memoryRead,
                                p.ioOptions);
                            p.thread.cache.add(fileName, read);
                        }
                    }
                    if (read)
                    {
                        videoData = read->readVideo(videoRequest->time).get();
                    }
                    videoRequest->promise.set_value(videoData);
                }
                
                // Handle audio requests.
                if (audioRequest)
                {
                    io::AudioData audioData;
                    std::shared_ptr<io::IRead> read;
                    const std::string& fileName = audioRequest->path.get();
                    if (!p.thread.cache.get(fileName, read))
                    {
                        if (auto context = p.context.lock())
                        {
                            auto ioSystem = context->getSystem<io::System>();
                            read = ioSystem->read(
                                infoRequest->path,
                                infoRequest->memoryRead,
                                p.ioOptions);
                            p.thread.cache.add(fileName, read);
                        }
                    }
                    if (read)
                    {
                        audioData = read->readAudio(audioRequest->range).get();
                    }
                    audioRequest->promise.set_value(audioData);
                }
            }
        }
        
        void IOManager::_cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            std::list<std::shared_ptr<Private::AudioRequest> > audioRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                videoRequests = std::move(p.mutex.videoRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
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
    }
}
