// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/SequenceIO.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/LRUCache.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <fseq.h>

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <queue>
#include <list>
#include <mutex>
#include <sstream>
#include <thread>

namespace tl
{
    namespace io
    {
        struct ISequenceRead::Private
        {
            void addTags(Info&);

            std::promise<Info> infoPromise;

            struct VideoRequest
            {
                VideoRequest() {}
                VideoRequest(VideoRequest&&) = default;

                otime::RationalTime time = time::invalidTime;
                uint16_t layer = 0;
                std::promise<VideoData> promise;

                std::string fileName;
                std::future<VideoData> future;
            };
            std::list<std::shared_ptr<VideoRequest> > videoRequests;
            std::list<std::shared_ptr<VideoRequest> > videoRequestsInProgress;
            std::condition_variable requestCV;

            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running;
            bool stopped = false;
            size_t threadCount = sequenceThreadCount;

            std::chrono::steady_clock::time_point logTimer;
        };

        void ISequenceRead::_init(
            const file::Path& path,
            const std::vector<MemoryRead>& memory,
            const Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);

            TLRENDER_P();

            const std::string& number = path.getNumber();
            if (!number.empty())
            {
                if (!_memory.empty())
                {
                    std::stringstream ss(number);
                    ss >> _startFrame;
                    _endFrame = _startFrame + _memory.size() - 1;
                }
                else
                {
                    std::string directory = path.getDirectory();
                    if (directory.empty())
                    {
                        directory = ".";
                    }
                    const std::string& baseName = path.getBaseName();
                    const std::string& extension = path.getExtension();
                    FSeqDirOptions dirOptions;
                    fseqDirOptionsInit(&dirOptions);
                    dirOptions.sequence = FSEQ_TRUE;
                    FSeqBool error = FSEQ_FALSE;
                    auto dirList = fseqDirList(directory.c_str(), &dirOptions, &error);
                    if (FSEQ_FALSE == error)
                    {
                        for (auto entry = dirList; entry; entry = entry->next)
                        {
                            if (strlen(entry->fileName.number) > 0 &&
                                0 == strcmp(entry->fileName.base, baseName.c_str()) &&
                                0 == strcmp(entry->fileName.extension, extension.c_str()))
                            {
                                _startFrame = entry->frameMin;
                                _endFrame = entry->frameMax;
                                break;
                            }
                        }
                    }
                    fseqDirListDel(dirList);
                }
            }

            auto i = options.find("SequenceIO/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.threadCount;
            }
            i = options.find("SequenceIO/DefaultSpeed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _defaultSpeed;
            }

            p.running = true;
            p.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        auto info = _getInfo(
                            path.get(), 
                            !_memory.empty() ? &_memory[0] : nullptr);
                        p.addTags(info);
                        p.infoPromise.set_value(info);
                        try
                        {
                            _run();
                        }
                        catch (const std::exception& e)
                        {
                            //! \todo How should this be handled?
                            if (auto logSystem = _logSystem.lock())
                            {
                                const std::string id = string::Format("tl::io::ISequenceRead ({0}: {1})").
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
                        //! \todo How should this be handled?

                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = string::Format("tl::io::ISequenceRead ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, string::Format("{0}: {1}").
                                arg(_path.get()).
                                arg(e.what()),
                                log::Type::Error);
                        }
                        p.infoPromise.set_value(Info());
                    }

                    {
                        std::list<std::shared_ptr<Private::VideoRequest> > videoRequestsCleanup;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            p.stopped = true;
                            videoRequestsCleanup = std::move(p.videoRequests);
                        }
                        videoRequestsCleanup.insert(
                            videoRequestsCleanup.end(),
                            p.videoRequestsInProgress.begin(),
                            p.videoRequestsInProgress.end());
                        for (auto& request : videoRequestsCleanup)
                        {
                            VideoData data;
                            data.time = request->time;
                            if (request->future.valid())
                            {
                                data = request->future.get();
                            }
                            request->promise.set_value(data);
                        }
                    }
                });
        }

        ISequenceRead::ISequenceRead() :
            _p(new Private)
        {}

        ISequenceRead::~ISequenceRead()
        {}

        std::future<Info> ISequenceRead::getInfo()
        {
            return _p->infoPromise.get_future();
        }

        std::future<VideoData> ISequenceRead::readVideo(
            const otime::RationalTime& time,
            uint16_t layer)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->layer = layer;
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
                request->promise.set_value(VideoData());
            }
            return future;
        }

        bool ISequenceRead::hasRequests()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return !p.videoRequests.empty();
        }

        void ISequenceRead::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                videoRequests = std::move(p.videoRequests);
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(VideoData());
            }
        }

        void ISequenceRead::stop()
        {
            _p->running = false;
        }

        bool ISequenceRead::hasStopped() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.stopped;
        }

        void ISequenceRead::_finish()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        void ISequenceRead::_run()
        {
            TLRENDER_P();
            p.logTimer = std::chrono::steady_clock::now();
            while (p.running)
            {
                // Gather requests.
                std::list<std::shared_ptr<Private::VideoRequest> > newVideoRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.requestCV.wait_for(
                        lock,
                        sequenceRequestTimeout,
                        [this]
                        {
                            return !_p->videoRequests.empty() || !_p->videoRequestsInProgress.empty();
                        }))
                    {
                        while (!p.videoRequests.empty() &&
                            (p.videoRequestsInProgress.size() + newVideoRequests.size()) < p.threadCount)
                        {
                            newVideoRequests.push_back(p.videoRequests.front());
                            p.videoRequests.pop_front();
                        }
                    }
                }

                // Initialize new requests.
                while (!newVideoRequests.empty())
                {
                    auto request = newVideoRequests.front();
                    newVideoRequests.pop_front();

                    //std::cout << "request: " << request.time << std::endl;
                    if (!_path.getNumber().empty())
                    {
                        request->fileName = _path.get(static_cast<int>(request->time.value()));
                    }
                    else
                    {
                        request->fileName = _path.get();
                    }
                    const std::string fileName = request->fileName;
                    const otime::RationalTime time = request->time;
                    const uint16_t layer = request->layer;
                    request->future = std::async(
                        std::launch::async,
                        [this, fileName, time, layer]
                        {
                            VideoData out;
                            try
                            {
                                const int64_t frame = time.value();
                                if (frame >= _startFrame && frame <= _endFrame)
                                {
                                    out = _readVideo(
                                        fileName,
                                        frame >= 0 && frame < _memory.size() ? &_memory[frame] : nullptr,
                                        time,
                                        layer);
                                }
                            }
                            catch (const std::exception&)
                            {
                                //! \todo How should this be handled?
                            }
                            return out;
                        });
                    p.videoRequestsInProgress.push_back(request);
                }

                // Check for finished requests.
                //if (!p.videoRequestsInProgress.empty())
                //{
                //    std::cout << "in progress: " << p.videoRequestsInProgress.size() << std::endl;
                //}
                auto requestIt = p.videoRequestsInProgress.begin();
                while (requestIt != p.videoRequestsInProgress.end())
                {
                    if ((*requestIt)->future.valid() &&
                        (*requestIt)->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        //std::cout << "finished: " << requestIt->time << std::endl;
                        auto data = (*requestIt)->future.get();
                        (*requestIt)->promise.set_value(data);
                        requestIt = p.videoRequestsInProgress.erase(requestIt);
                        continue;
                    }
                    ++requestIt;
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.logTimer = now;
                        const std::string id = string::Format("tl::io::ISequenceRead {0}").arg(this);
                        size_t videoRequestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            videoRequestsSize = p.videoRequests.size();
                        }
                        logSystem->print(id, string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Video requests: {1}, {2} in progress\n"
                            "    Thread count: {3}").
                            arg(_path.get()).
                            arg(videoRequestsSize).
                            arg(p.videoRequestsInProgress.size()).
                            arg(p.threadCount));
                    }
                }
            }
        }

        void ISequenceRead::Private::addTags(Info& info)
        {
            if (!info.video.empty())
            {
                {
                    std::stringstream ss;
                    ss << info.video[0].size.w << " " << info.video[0].size.h;
                    info.tags["Video Resolution"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << info.video[0].pixelAspectRatio;
                    info.tags["Video Pixel Aspect Ratio"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].pixelType;
                    info.tags["Video Pixel Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].videoLevels;
                    info.tags["Video Levels"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.videoTime.start_time().to_timecode();
                    info.tags["Video Start Time"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.videoTime.duration().to_timecode();
                    info.tags["Video Duration"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << info.videoTime.start_time().rate() << " FPS";
                    info.tags["Video Speed"] = ss.str();
                }
            }
        }

        struct ISequenceWrite::Private
        {
            std::string path;
            std::string baseName;
            std::string number;
            int pad = 0;
            std::string extension;

            float defaultSpeed = sequenceDefaultSpeed;
        };

        void ISequenceWrite::_init(
            const file::Path& path,
            const Info& info,
            const Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);

            TLRENDER_P();

            const auto i = options.find("SequenceIO/DefaultSpeed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.defaultSpeed;
            }
        }

        ISequenceWrite::ISequenceWrite() :
            _p(new Private)
        {}

        ISequenceWrite::~ISequenceWrite()
        {}

        void ISequenceWrite::writeVideo(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            _writeVideo(_path.get(static_cast<int>(time.value())), time, image);
        }
    }
}
