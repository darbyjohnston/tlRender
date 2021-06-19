// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/SequenceIO.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>

#include <iomanip>
#include <sstream>

namespace tlr
{
    namespace io
    {
        void ISequenceRead::_init(
            const std::string& fileName,
            const Options& options)
        {
            IRead::_init(fileName, options);

            file::split(fileName, &_path, &_baseName, &_number, &_extension);
            _pad = !_number.empty() ? ('0' == _number[0] ? _number.size() : 0) : 0;

            _videoFrameCache.setMax(1);

            _running = true;
            _stopped = false;
            _thread = std::thread(
                [this, fileName]
                {
                    try
                    {
                        _infoPromise.set_value(_getInfo(fileName));
                        _run();
                    }
                    catch (const std::exception&)
                    {
                        _infoPromise.set_value(Info());
                    }
                    _stopped = true;
                    std::list<VideoFrameRequest> videoFrameRequests;
                    {
                        std::unique_lock<std::mutex> lock(_requestMutex);
                        videoFrameRequests.swap(_videoFrameRequests);
                    }
                    for (auto& i : videoFrameRequests)
                    {
                        i.promise.set_value(VideoFrame());
                    }
                });
        }

        ISequenceRead::ISequenceRead()
        {}

        ISequenceRead::~ISequenceRead()
        {
            _running = false;
            if (_thread.joinable())
            {
                _thread.join();
            }
        }

        std::future<Info> ISequenceRead::getInfo()
        {
            return _infoPromise.get_future();
        }

        std::future<VideoFrame> ISequenceRead::readVideoFrame(const otime::RationalTime& time)
        {
            VideoFrameRequest request;
            request.time = time;
            auto future = request.promise.get_future();
            if (!_stopped)
            {
                {
                    std::unique_lock<std::mutex> lock(_requestMutex);
                    _videoFrameRequests.push_back(std::move(request));
                }
                _requestCV.notify_one();
            }
            else
            {
                request.promise.set_value(VideoFrame());
            }
            return future;
        }

        bool ISequenceRead::hasVideoFrames()
        {
            std::unique_lock<std::mutex> lock(_requestMutex);
            return !_videoFrameRequests.empty();
        }

        void ISequenceRead::cancelVideoFrames()
        {
            std::unique_lock<std::mutex> lock(_requestMutex);
            _videoFrameRequests.clear();
        }

        void ISequenceRead::stop()
        {
            _running = false;
        }

        bool ISequenceRead::hasStopped() const
        {
            return _stopped;
        }

        void ISequenceRead::_run()
        {
            while (_running)
            {
                struct Result
                {
                    std::string fileName;
                    otime::RationalTime time = invalidTime;
                    std::future<VideoFrame> future;
                    std::promise<VideoFrame> promise;
                };
                std::vector<Result> results;
                {
                    std::unique_lock<std::mutex> lock(_requestMutex);
                    _requestCV.wait_for(
                        lock,
                        sequenceRequestTimeout,
                        [this]
                        {
                            return !_videoFrameRequests.empty();
                        });
                    for (size_t i = 0; i < sequenceThreadCount && !_videoFrameRequests.empty(); ++i)
                    {
                        Result result;
                        result.time = _videoFrameRequests.front().time;
                        result.promise = std::move(_videoFrameRequests.front().promise);
                        results.push_back(std::move(result));
                        _videoFrameRequests.pop_front();
                    }
                }

                auto it = results.begin();
                while (it != results.end())
                {
                    //std::cout << "request: " << it->time << std::endl;
                    std::stringstream ss;
                    if (!_number.empty())
                    {
                        ss << _path << _baseName << std::setfill('0') << std::setw(_pad) << static_cast<int>(it->time.value()) << _extension;
                    }
                    else
                    {
                        ss << _fileName;
                    }
                    it->fileName = ss.str();
                    VideoFrame videoFrame;
                    if (_videoFrameCache.get(it->fileName, videoFrame))
                    {
                        it->promise.set_value(videoFrame);
                        it = results.erase(it);
                    }
                    else
                    {
                        const auto fileName = it->fileName;
                        const auto time = it->time;
                        it->future = std::async(
                            std::launch::async,
                            [this, fileName, time]
                            {
                                VideoFrame out;
                                try
                                {
                                    out = _readVideoFrame(fileName, time);
                                }
                                catch (const std::exception&e)
                                {}
                                return out;
                            });
                        ++it;
                    }
                }
                for (auto& i : results)
                {
                    auto videoFrame = i.future.get();
                    i.promise.set_value(videoFrame);
                    _videoFrameCache.add(i.fileName, videoFrame);
                }
            }
        }

        void ISequenceWrite::_init(
            const std::string& fileName,
            const Info& info,
            const Options& options)
        {
            IWrite::_init(fileName, options, info);

            file::split(fileName, &_path, &_baseName, &_number, &_extension);
            _pad = !_number.empty() ? ('0' == _number[0] ? _number.size() : 0) : 0;
        }

        ISequenceWrite::ISequenceWrite()
        {}

        ISequenceWrite::~ISequenceWrite()
        {}

        void ISequenceWrite::writeVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            std::stringstream ss;
            ss << _path << _baseName << std::setfill('0') << std::setw(_pad) << static_cast<int>(time.value()) << _extension;
            _writeVideoFrame(ss.str(), time, image);
        }
    }
}
