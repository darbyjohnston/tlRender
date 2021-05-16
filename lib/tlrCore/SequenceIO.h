// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/IO.h>

#include <atomic>
#include <condition_variable>
#include <queue>
#include <list>
#include <mutex>
#include <thread>

namespace tlr
{
    namespace io
    {
        //! Base class for image sequence readers.
        class ISequenceRead : public IRead
        {
        protected:
            void _init(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed);
            ISequenceRead();

        public:
            ~ISequenceRead() override;

            std::future<io::Info> getInfo() override;
            std::future<io::VideoFrame> getVideoFrame(const otime::RationalTime&) override;
            void cancelVideoFrames() override;

        protected:
            std::string _getFileName(const otime::RationalTime&) const;

            virtual io::Info _getInfo(const std::string& fileName) = 0;
            virtual io::VideoFrame _getVideoFrame(const otime::RationalTime&) = 0;

            std::string _path;
            std::string _baseName;
            std::string _number;
            int _pad = 0;
            std::string _extension;

        private:
            void _run();

            std::promise<io::Info> _infoPromise;
            struct VideoFrameRequest
            {
                VideoFrameRequest() {}
                VideoFrameRequest(VideoFrameRequest&& other) = default;

                otime::RationalTime time;
                std::promise<io::VideoFrame> promise;
            };
            std::list<VideoFrameRequest> _videoFrameRequests;
            std::condition_variable _requestCV;
            std::mutex _requestMutex;

            std::thread _thread;
            std::atomic<bool> _running;
        };
    }
}
