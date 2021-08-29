// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/SequenceIO.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>
#include <tlrCore/LRUCache.h>

#include <atomic>
#include <condition_variable>
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

            struct VideoFrameRequest
            {
                VideoFrameRequest() {}
                VideoFrameRequest(VideoFrameRequest&&) = default;

                otime::RationalTime time = time::invalidTime;
                std::shared_ptr<imaging::Image> image;
                std::promise<VideoFrame> promise;

                std::string fileName;
                std::future<VideoFrame> future;
            };
            std::list<VideoFrameRequest> videoFrameRequests;
            std::list<VideoFrameRequest> videoFrameRequestsInProgress;
            std::condition_variable requestCV;
            std::mutex requestMutex;

            std::thread thread;
            std::atomic<bool> running;
            std::atomic<bool> stopped;
            size_t threadCount = sequenceThreadCount;
        };

        void ISequenceRead::_init(
            const file::Path& path,
            const Options& options,
            const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IRead::_init(path, options, logSystem);

            TLR_PRIVATE_P();

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
            p.stopped = false;
            p.thread = std::thread(
                [this, path]
                {
                    TLR_PRIVATE_P();
                    try
                    {
                        p.infoPromise.set_value(_getInfo(path.get()));
                        _run();
                    }
                    catch (const std::exception&)
                    {
                        p.infoPromise.set_value(Info());
                    }
                    p.stopped = true;
                    std::list<Private::VideoFrameRequest> videoFrameRequests;
                    {
                        std::unique_lock<std::mutex> lock(p.requestMutex);
                        videoFrameRequests.swap(p.videoFrameRequests);
                    }
                    for (auto& i : videoFrameRequests)
                    {
                        i.promise.set_value(VideoFrame());
                    }
                });
        }

        ISequenceRead::ISequenceRead() :
            _p(new Private)
        {}

        ISequenceRead::~ISequenceRead()
        {
            TLR_PRIVATE_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::future<Info> ISequenceRead::getInfo()
        {
            return _p->infoPromise.get_future();
        }

        std::future<VideoFrame> ISequenceRead::readVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            TLR_PRIVATE_P();
            Private::VideoFrameRequest request;
            request.time = time;
            request.image = image;
            auto future = request.promise.get_future();
            if (!p.stopped)
            {
                {
                    std::unique_lock<std::mutex> lock(p.requestMutex);
                    p.videoFrameRequests.push_back(std::move(request));
                }
                p.requestCV.notify_one();
            }
            else
            {
                request.promise.set_value(VideoFrame());
            }
            return future;
        }

        bool ISequenceRead::hasVideoFrames()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.requestMutex);
            return !p.videoFrameRequests.empty();
        }

        void ISequenceRead::cancelVideoFrames()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.requestMutex);
            p.videoFrameRequests.clear();
        }

        void ISequenceRead::stop()
        {
            _p->running = false;
        }

        bool ISequenceRead::hasStopped() const
        {
            return _p->stopped;
        }

        void ISequenceRead::_run()
        {
            TLR_PRIVATE_P();
            while (p.running)
            {
                // Gather requests.
                std::list<Private::VideoFrameRequest> newVideoFrameRequests;
                {
                    std::unique_lock<std::mutex> lock(p.requestMutex);
                    p.requestCV.wait_for(
                        lock,
                        sequenceRequestTimeout,
                        [this]
                        {
                            return !_p->videoFrameRequests.empty() || !_p->videoFrameRequestsInProgress.empty();
                        });
                    while (!p.videoFrameRequests.empty() &&
                        (p.videoFrameRequestsInProgress.size() + newVideoFrameRequests.size()) < p.threadCount)
                    {
                        newVideoFrameRequests.push_back(std::move(p.videoFrameRequests.front()));
                        p.videoFrameRequests.pop_front();
                    }
                }

                // Iniitalize new requests.
                for (auto& request : newVideoFrameRequests)
                {
                    //std::cout << "request: " << it->time << std::endl;
                    if (!_path.getNumber().empty())
                    {
                        request.fileName = _path.get(static_cast<int>(request.time.value()));
                    }
                    else
                    {
                        request.fileName = _path.get();
                    }
                    const auto fileName = request.fileName;
                    const auto time = request.time;
                    const auto image = request.image;
                    request.future = std::async(
                        std::launch::async,
                        [this, fileName, time, image]
                        {
                            VideoFrame out;
                            try
                            {
                                out = _readVideoFrame(fileName, time, image);
                            }
                            catch (const std::exception&)
                            {
                            }
                            return out;
                        });
                    p.videoFrameRequestsInProgress.push_back(std::move(request));
                }

                // Check for finished requests.
                auto requestIt = p.videoFrameRequestsInProgress.begin();
                while (requestIt != p.videoFrameRequestsInProgress.end())
                {
                    if (requestIt->future.valid() &&
                        requestIt->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        auto videoFrame = requestIt->future.get();
                        requestIt->promise.set_value(videoFrame);
                        requestIt = p.videoFrameRequestsInProgress.erase(requestIt);
                        continue;
                    }
                    ++requestIt;
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

        void ISequenceWrite::writeVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            _writeVideoFrame(_path.get(static_cast<int>(time.value())), time, image);
        }
    }
}
