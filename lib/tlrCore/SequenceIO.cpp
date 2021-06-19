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
            const io::Options& options)
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
                        try
                        {
                            _run();
                        }
                        catch (const std::exception&)
                        {}
                    }
                    catch (const std::exception&)
                    {
                        _infoPromise.set_value(io::Info());
                    }
                    _stopped = true;
                    std::list<VideoFrameRequest> videoFrameRequests;
                    {
                        std::unique_lock<std::mutex> lock(_requestMutex);
                        videoFrameRequests.swap(_videoFrameRequests);
                    }
                    for (auto& i : videoFrameRequests)
                    {
                        i.promise.set_value(io::VideoFrame());
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

        std::future<io::Info> ISequenceRead::getInfo()
        {
            return _infoPromise.get_future();
        }

        std::future<io::VideoFrame> ISequenceRead::readVideoFrame(const otime::RationalTime& time)
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
                request.promise.set_value(io::VideoFrame());
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
                VideoFrameRequest request;
                bool requestValid = false;
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
                        request.time = _videoFrameRequests.front().time;
                        request.promise = std::move(_videoFrameRequests.front().promise);
                        _videoFrameRequests.pop_front();
                        requestValid = true;
                    }
                }
                if (requestValid)
                {
                    //std::cout << "request: " << i.time << std::endl;
                    std::stringstream ss;
                    if (!_number.empty())
                    {
                        ss << _path << _baseName << std::setfill('0') << std::setw(_pad) << static_cast<int>(request.time.value()) << _extension;
                    }
                    else
                    {
                        ss << _fileName;
                    }
                    std::string fileName = ss.str();
                    io::VideoFrame videoFrame;
                    if (_videoFrameCache.get(fileName, videoFrame))
                    {
                        request.promise.set_value(videoFrame);
                    }
                    else
                    {
                        videoFrame = _readVideoFrame(fileName, request.time);
                        request.promise.set_value(videoFrame);
                        _videoFrameCache.add(fileName, videoFrame);
                    }
                }
            }
        }

        void ISequenceWrite::_init(
            const std::string& fileName,
            const io::Info& info,
            const io::Options& options)
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
