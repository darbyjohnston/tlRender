// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Cache.h>
#include <tlrCore/IO.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

} // extern "C"

#include <atomic>
#include <condition_variable>
#include <queue>
#include <list>
#include <map>
#include <mutex>
#include <thread>

namespace tlr
{
    //! FFmpeg I/O
    namespace ffmpeg
    {
        //! Number of threads.
        const size_t threadCount = 4;

        //! Timeout for frame requests.
        const std::chrono::microseconds requestTimeout(1000);

        //! Software scaler flags.
        const int swsScaleFlags = SWS_FAST_BILINEAR;

        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! FFmpeg reader
        class Read : public io::IRead
        {
        protected:
            void _init(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoFrame> readVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;
            bool hasVideoFrames() override;
            void cancelVideoFrames() override;

        private:
            void _open(const std::string& fileName);
            void _run();
            void _close();
            int _decodeVideo(
                AVPacket&,
                io::VideoFrame&,
                bool hasSeek,
                const otime::RationalTime& seek);
            void _copyVideo(const std::shared_ptr<imaging::Image>&);

            io::Info _info;
            std::promise<io::Info> _infoPromise;
            struct VideoFrameRequest
            {
                VideoFrameRequest() {}
                VideoFrameRequest(VideoFrameRequest&& other) = default;

                otime::RationalTime time;
                std::shared_ptr<imaging::Image> image;
                std::promise<io::VideoFrame> promise;
            };
            std::list<VideoFrameRequest> _videoFrameRequests;
            std::condition_variable _requestCV;
            std::mutex _requestMutex;
            otime::RationalTime _currentTime;
            memory::Cache<otime::RationalTime, io::VideoFrame> _videoFrameCache;

            AVFormatContext* _avFormatContext = nullptr;
            int _avVideoStream = -1;
            std::map<int, AVCodecParameters*> _avCodecParameters;
            std::map<int, AVCodecContext*> _avCodecContext;
            AVFrame* _avFrame = nullptr;
            AVFrame* _avFrameRgb = nullptr;
            SwsContext* _swsContext = nullptr;

            std::thread _thread;
            std::atomic<bool> _running;
        };

        //! FFmpeg writer.
        class Write : public io::IWrite
        {
        protected:
            void _init(
                const std::string& fileName,
                const io::Info&);
            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const std::string& fileName,
                const io::Info&);

            void writeVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;

        private:
        };

        //! FFmpeg Plugin
        class Plugin : public io::IPlugin
        {
        protected:
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create();

            std::shared_ptr<io::IRead> read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed) override;
            std::vector<imaging::PixelType> getWritePixelTypes() const override;
            std::shared_ptr<io::IWrite> write(
                const std::string& fileName,
                const io::Info&) override;
        };
    }
}
