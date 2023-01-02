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

            size_t threadCount = sequenceThreadCount;

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
            struct VideoMutex
            {
                std::list<std::shared_ptr<VideoRequest> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            VideoMutex videoMutex;
            struct VideoThread
            {
                std::list<std::shared_ptr<VideoRequest> > requestsInProgress;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            VideoThread videoThread;
        };

        void ISequenceRead::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
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

            p.videoThread.running = true;
            p.videoThread.thread = std::thread(
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
                            _videoThread();
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
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        p.videoMutex.stopped = true;
                    }
                    _cancelVideoRequests();

                    for (auto& request : p.videoThread.requestsInProgress)
                    {
                        VideoData data;
                        data.time = request->time;
                        if (request->future.valid())
                        {
                            data = request->future.get();
                        }
                        request->promise.set_value(data);
                    }
                    p.videoThread.requestsInProgress.clear();
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
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.stopped)
                {
                    valid = true;
                    p.videoMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.videoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(VideoData());
            }
            return future;
        }

        void ISequenceRead::cancelRequests()
        {
            _cancelVideoRequests();
        }

        void ISequenceRead::_finish()
        {
            TLRENDER_P();
            p.videoThread.running = false;
            if (p.videoThread.thread.joinable())
            {
                p.videoThread.thread.join();
            }
        }

        void ISequenceRead::_videoThread()
        {
            TLRENDER_P();
            p.videoThread.logTimer = std::chrono::steady_clock::now();
            while (p.videoThread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::VideoRequest> > newVideoRequests;
                {
                    std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                    if (p.videoThread.cv.wait_for(
                        lock,
                        sequenceRequestTimeout,
                        [this]
                        {
                            return
                                !_p->videoMutex.requests.empty() ||
                                !_p->videoThread.requestsInProgress.empty();
                        }))
                    {
                        while (!p.videoMutex.requests.empty() &&
                            (p.videoThread.requestsInProgress.size() + newVideoRequests.size()) < p.threadCount)
                        {
                            newVideoRequests.push_back(p.videoMutex.requests.front());
                            p.videoMutex.requests.pop_front();
                        }
                    }
                }

                // Initialize new requests.
                while (!newVideoRequests.empty())
                {
                    auto request = newVideoRequests.front();
                    newVideoRequests.pop_front();

                    //std::cout << "request: " << request.time << std::endl;
                    bool seq = false;
                    if (!_path.getNumber().empty())
                    {
                        seq = true;
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
                        [this, seq, fileName, time, layer]
                        {
                            VideoData out;
                            try
                            {
                                const int64_t frame = time.value();
                                if (!seq || (seq && frame >= _startFrame && frame <= _endFrame))
                                {
                                    const int64_t memoryIndex = seq ? (frame - _startFrame) : 0;
                                    out = _readVideo(
                                        fileName,
                                        memoryIndex >= 0 && memoryIndex < _memory.size() ? &_memory[memoryIndex] : nullptr,
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
                    p.videoThread.requestsInProgress.push_back(request);
                }

                // Check for finished requests.
                //if (!p.videoThread.requestsInProgress.empty())
                //{
                //    std::cout << "in progress: " << p.videoThread.requestsInProgress.size() << std::endl;
                //}
                auto requestIt = p.videoThread.requestsInProgress.begin();
                while (requestIt != p.videoThread.requestsInProgress.end())
                {
                    if ((*requestIt)->future.valid() &&
                        (*requestIt)->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        //std::cout << "finished: " << requestIt->time << std::endl;
                        auto data = (*requestIt)->future.get();
                        (*requestIt)->promise.set_value(data);
                        requestIt = p.videoThread.requestsInProgress.erase(requestIt);
                        continue;
                    }
                    ++requestIt;
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.videoThread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.videoThread.logTimer = now;
                        const std::string id = string::Format("tl::io::ISequenceRead {0}").arg(this);
                        size_t videoRequestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                            videoRequestsSize = p.videoMutex.requests.size();
                        }
                        logSystem->print(id, string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Video requests: {1}, {2} in progress\n"
                            "    Thread count: {3}").
                            arg(_path.get()).
                            arg(videoRequestsSize).
                            arg(p.videoThread.requestsInProgress.size()).
                            arg(p.threadCount));
                    }
                }
            }
        }

        void ISequenceRead::_cancelVideoRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                requests = std::move(p.videoMutex.requests);
            }
            for (auto& request : requests)
            {
                request->promise.set_value(VideoData());
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
                    ss << info.video[0].size.pixelAspectRatio;
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
