// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/SequenceIO.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>
#include <tlrCore/LRUCache.h>

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

            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running;
            bool stopped = false;
            size_t threadCount = sequenceThreadCount;
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
                const std::string& directory = path.getDirectory();
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

                    std::list<Private::VideoFrameRequest> videoFrameRequestsCleanup;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.stopped = true;
                        while (!p.videoFrameRequests.empty())
                        {
                            videoFrameRequestsCleanup.push_back(std::move(p.videoFrameRequests.front()));
                            p.videoFrameRequests.pop_front();
                        }
                    }
                    while (!p.videoFrameRequestsInProgress.empty())
                    {
                        videoFrameRequestsCleanup.push_back(std::move(p.videoFrameRequestsInProgress.front()));
                        p.videoFrameRequestsInProgress.pop_front();
                    }
                    for (auto& request : videoFrameRequestsCleanup)
                    {
                        VideoFrame frame;
                        frame.time = request.time;
                        if (request.future.valid())
                        {
                            frame = request.future.get();
                        }
                        request.promise.set_value(frame);
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

        std::future<VideoFrame> ISequenceRead::readVideoFrame(
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
                request.promise.set_value(VideoFrame());
            }
            return future;
        }

        bool ISequenceRead::hasVideoFrames()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return !p.videoFrameRequests.empty();
        }

        void ISequenceRead::cancelVideoFrames()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.videoFrameRequests.clear();
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
            while (p.running)
            {
                // Gather requests.
                std::list<Private::VideoFrameRequest> newVideoFrameRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    if (p.requestCV.wait_for(
                        lock,
                        sequenceRequestTimeout,
                        [this]
                        {
                            return !_p->videoFrameRequests.empty() || !_p->videoFrameRequestsInProgress.empty();
                        }))
                    {
                        while (!p.videoFrameRequests.empty() &&
                            (p.videoFrameRequestsInProgress.size() + newVideoFrameRequests.size()) < p.threadCount)
                        {
                            newVideoFrameRequests.push_back(std::move(p.videoFrameRequests.front()));
                            p.videoFrameRequests.pop_front();
                        }
                    }
                }

                // Iniitalize new requests.
                while (!newVideoFrameRequests.empty())
                {
                    auto request = std::move(newVideoFrameRequests.front());
                    newVideoFrameRequests.pop_front();
                    
                    //std::cout << "request: " << request.time << std::endl;
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
                //if (!p.videoFrameRequestsInProgress.empty())
                //{
                //    std::cout << "in progress: " << p.videoFrameRequestsInProgress.size() << std::endl;
                //}
                auto requestIt = p.videoFrameRequestsInProgress.begin();
                while (requestIt != p.videoFrameRequestsInProgress.end())
                {
                    if (requestIt->future.valid() &&
                        requestIt->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        //std::cout << "finished: " << requestIt->time << std::endl;
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
