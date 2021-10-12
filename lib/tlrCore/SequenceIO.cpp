// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/SequenceIO.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>
#include <tlrCore/LRUCache.h>
#include <tlrCore/LogSystem.h>
#include <tlrCore/StringFormat.h>

#include <fseq.h>

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <queue>
#include <list>
#include <mutex>
#include <sstream>
#include <thread>

namespace tlr
{
    namespace avio
    {
        struct ISequenceRead::Private
        {
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
            std::list<VideoRequest> videoRequests;
            std::list<VideoRequest> videoRequestsInProgress;
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
            const Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IRead::_init(path, options, logSystem);

            TLR_PRIVATE_P();

            const std::string& number = path.getNumber();
            if (!number.empty())
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
                        if (0 == strcmp(entry->fileName.base, baseName.c_str()) &&
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
                    TLR_PRIVATE_P();
                    try
                    {
                        p.infoPromise.set_value(_getInfo(path.get()));
                        try
                        {
                            _run();
                        }
                        catch (const std::exception&)
                        {}
                    }
                    catch (const std::exception&)
                    {
                        p.infoPromise.set_value(Info());
                    }

                    std::list<Private::VideoRequest> videoRequestsCleanup;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.stopped = true;
                        while (!p.videoRequests.empty())
                        {
                            videoRequestsCleanup.push_back(std::move(p.videoRequests.front()));
                            p.videoRequests.pop_front();
                        }
                    }
                    while (!p.videoRequestsInProgress.empty())
                    {
                        videoRequestsCleanup.push_back(std::move(p.videoRequestsInProgress.front()));
                        p.videoRequestsInProgress.pop_front();
                    }
                    for (auto& request : videoRequestsCleanup)
                    {
                        VideoData data;
                        data.time = request.time;
                        if (request.future.valid())
                        {
                            data = request.future.get();
                        }
                        request.promise.set_value(data);
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
            TLR_PRIVATE_P();
            Private::VideoRequest request;
            request.time = time;
            request.layer = layer;
            auto future = request.promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                if (!p.stopped)
                {
                    valid = true;
                    p.videoRequests.push_back(std::move(request));
                }
            }
            if (valid)
            {
                p.requestCV.notify_one();
            }
            else
            {
                request.promise.set_value(VideoData());
            }
            return future;
        }

        bool ISequenceRead::hasRequests()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return !p.videoRequests.empty();
        }

        void ISequenceRead::cancelRequests()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.videoRequests.clear();
        }

        void ISequenceRead::stop()
        {
            _p->running = false;
        }

        bool ISequenceRead::hasStopped() const
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.stopped;
        }
        
        void ISequenceRead::_finish()
        {
            TLR_PRIVATE_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        void ISequenceRead::_run()
        {
            TLR_PRIVATE_P();
            p.logTimer = std::chrono::steady_clock::now();
            while (p.running)
            {
                // Gather requests.
                std::list<Private::VideoRequest> newVideoRequests;
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
                            newVideoRequests.push_back(std::move(p.videoRequests.front()));
                            p.videoRequests.pop_front();
                        }
                    }
                }

                // Iniitalize new requests.
                while (!newVideoRequests.empty())
                {
                    auto request = std::move(newVideoRequests.front());
                    newVideoRequests.pop_front();
                    
                    //std::cout << "request: " << request.time << std::endl;
                    if (!_path.getNumber().empty())
                    {
                        request.fileName = _path.get(static_cast<int>(request.time.value()));
                    }
                    else
                    {
                        request.fileName = _path.get();
                    }
                    const std::string fileName = request.fileName;
                    const otime::RationalTime time = request.time;
                    const uint16_t layer = request.layer;
                    request.future = std::async(
                        std::launch::async,
                        [this, fileName, time, layer]
                        {
                            VideoData out;
                            try
                            {
                                out = _readVideo(fileName, time, layer);
                            }
                            catch (const std::exception&)
                            {
                            }
                            return out;
                        });
                    p.videoRequestsInProgress.push_back(std::move(request));
                }

                // Check for finished requests.
                //if (!p.videoRequestsInProgress.empty())
                //{
                //    std::cout << "in progress: " << p.videoRequestsInProgress.size() << std::endl;
                //}
                auto requestIt = p.videoRequestsInProgress.begin();
                while (requestIt != p.videoRequestsInProgress.end())
                {
                    if (requestIt->future.valid() &&
                        requestIt->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        //std::cout << "finished: " << requestIt->time << std::endl;
                        auto data = requestIt->future.get();
                        requestIt->promise.set_value(data);
                        requestIt = p.videoRequestsInProgress.erase(requestIt);
                        continue;
                    }
                    ++requestIt;
                }

                // Logging.
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - p.logTimer;
                if (diff.count() > 10.F)
                {
                    p.logTimer = now;
                    const std::string id = string::Format("tlr::avio::ISequenceRead {0}").arg(this);
                    size_t videoRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        videoRequestsSize = p.videoRequests.size();
                    }
                    _logSystem->print(id, string::Format(
                        "\n"
                        "    path: {0}\n"
                        "    video: {1}/{2} (requests/in progress)\n"
                        "    thread count: {3}").
                        arg(_path.get()).
                        arg(videoRequestsSize).
                        arg(p.videoRequestsInProgress.size()).
                        arg(p.threadCount));
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
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);

            TLR_PRIVATE_P();

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
