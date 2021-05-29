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
            const otime::RationalTime& defaultSpeed)
        {
            IRead::_init(fileName, defaultSpeed);

            file::split(fileName, &_path, &_baseName, &_number, &_extension);
            _pad = !_number.empty() ? ('0' == _number[0] ? _number.size() : 0) : 0;

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
                        //! \todo How should this be handled?
                        _infoPromise.set_value(io::Info());
                    }
                    _stopped = true;
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

        std::future<io::VideoFrame> ISequenceRead::readVideoFrame(
            const otime::RationalTime& time,
            const std::shared_ptr<imaging::Image>& image)
        {
            VideoFrameRequest request;
            request.time = time;
            request.image = image;
            auto future = request.promise.get_future();
            {
                std::unique_lock<std::mutex> lock(_requestMutex);
                _videoFrameRequests.push_back(std::move(request));
            }
            _requestCV.notify_one();
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
                std::list<VideoFrameRequest> requests;
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
                        VideoFrameRequest request;
                        request.time = _videoFrameRequests.front().time;
                        request.image = std::move(_videoFrameRequests.front().image);
                        request.promise = std::move(_videoFrameRequests.front().promise);
                        requests.push_back(std::move(request));
                        _videoFrameRequests.pop_front();
                    }
                }
                std::list<std::future<io::VideoFrame> > futures;
                for (const auto& i : requests)
                {
                    //std::cout << "request: " << i.time << std::endl;
                    std::stringstream ss;
                    ss << _path << _baseName << std::setfill('0') << std::setw(_pad) << static_cast<int>(i.time.value()) << _extension;
                    std::string fileName = ss.str();
                    futures.push_back(std::async(
                        std::launch::async,
                        [this, &i, fileName]
                        {
                            io::VideoFrame out;
                            try
                            {
                                out = _readVideoFrame(fileName, i.time, i.image);
                            }
                            catch (const std::exception&)
                            {
                                //! \todo How should this be handled?
                            }
                            return out;
                        }));
                }
                while (!requests.empty())
                {
                    const auto videoFrame = futures.front().get();
                    requests.front().promise.set_value(videoFrame);
                    requests.pop_front();
                    futures.pop_front();
                }
            }
        }

        void ISequenceWrite::_init(
            const std::string& fileName,
            const io::Info& info)
        {
            IWrite::_init(fileName, info);

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
