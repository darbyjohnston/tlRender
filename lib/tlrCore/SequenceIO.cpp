// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/SequenceIO.h>

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

        std::future<io::VideoFrame> ISequenceRead::getVideoFrame(const otime::RationalTime& time)
        {
            VideoFrameRequest request;
            request.time = time;
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

        std::string ISequenceRead::_getFileName(const otime::RationalTime& value) const
        {
            std::stringstream ss;
            ss << _path << _baseName << std::setfill('0') << std::setw(_pad) << static_cast<int>(value.value()) << _extension;
            return ss.str();
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
                        std::chrono::microseconds(1000),
                        [this]
                        {
                            return !_videoFrameRequests.empty();
                        });
                    if (!_videoFrameRequests.empty())
                    {
                        request.time = _videoFrameRequests.front().time;
                        request.promise = std::move(_videoFrameRequests.front().promise);
                        _videoFrameRequests.pop_front();
                        requestValid = true;
                    }
                }
                if (requestValid)
                {
                    //std::cout << "request: " << request.time << std::endl;
                    request.promise.set_value(_getVideoFrame(request.time));
                }
            }
        }
    }
}
