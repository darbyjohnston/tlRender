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
        //! Number of threads.
        const size_t sequenceThreadCount = 1;

        //! Timeout for frame requests.
        const std::chrono::microseconds sequenceRequestTimeout(1000);

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
            std::future<io::VideoFrame> readVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;
            bool hasVideoFrames() override;
            void cancelVideoFrames() override;
            void stop() override;
            bool hasStopped() const override;

        protected:
            virtual io::Info _getInfo(const std::string& fileName) = 0;
            virtual io::VideoFrame _readVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) = 0;

        private:
            void _run();

            std::string _path;
            std::string _baseName;
            std::string _number;
            int _pad = 0;
            std::string _extension;
            std::promise<io::Info> _infoPromise;
            struct VideoFrameRequest
            {
                VideoFrameRequest() {}
                VideoFrameRequest(VideoFrameRequest&& other) = default;

                otime::RationalTime time = invalidTime;
                std::shared_ptr<imaging::Image> image;
                std::promise<io::VideoFrame> promise;
            };
            std::list<VideoFrameRequest> _videoFrameRequests;
            std::condition_variable _requestCV;
            std::mutex _requestMutex;
            std::thread _thread;
            std::atomic<bool> _running;
            std::atomic<bool> _stopped;
        };

        //! Base class for image sequence writers.
        class ISequenceWrite : public IWrite
        {
        protected:
            void _init(
                const std::string& fileName,
                const io::Info&);
            ISequenceWrite();

        public:
            ~ISequenceWrite() override;

            void writeVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;

        protected:
            virtual void _writeVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) = 0;

        private:
            std::string _path;
            std::string _baseName;
            std::string _number;
            int _pad = 0;
            std::string _extension;
        };
    }
}
