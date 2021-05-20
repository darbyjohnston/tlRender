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
        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! FFmpeg Reader
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
            std::future<io::VideoFrame> getVideoFrame(const otime::RationalTime&) override;
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

        //! FFmpeg Plugin
        class Plugin : public io::IPlugin
        {
        protected:
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create();

            //bool canRead(const std::string&) override;
            std::shared_ptr<io::IRead> read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed) override;
        };
    }
}
