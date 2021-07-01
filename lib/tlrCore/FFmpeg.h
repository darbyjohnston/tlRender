// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AVIO.h>

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
        //! Profiles.
        enum class Profile
        {
            H264,
            ProRes,
            ProRes_Proxy,
            ProRes_LT,
            ProRes_HQ,
            ProRes_4444,
            ProRes_XQ,

            Count
        };
        TLR_ENUM(Profile);

        //! Number of threads.
        const size_t threadCount = 4;

        //! Timeout for frame requests.
        const std::chrono::microseconds requestTimeout(1000);

        //! Software scaler flags.
        const int swsScaleFlags = SWS_FAST_BILINEAR;

        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! Swap the numerator and denominator.
        AVRational swap(AVRational);

        //! FFmpeg reader
        class Read : public avio::IRead
        {
        protected:
            void _init(
                const std::string& fileName,
                const avio::Options&);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const std::string& fileName,
                const avio::Options&);

            std::future<avio::Info> getInfo() override;
            std::future<avio::VideoFrame> readVideoFrame(const otime::RationalTime&) override;
            bool hasVideoFrames() override;
            void cancelVideoFrames() override;
            void stop() override;
            bool hasStopped() const override;

        private:
            void _open(const std::string& fileName);
            void _run();
            void _close();
            int _decodeVideo(AVPacket*, const otime::RationalTime& seek);
            void _copyVideo(const std::shared_ptr<imaging::Image>&);

            avio::Info _info;
            std::promise<avio::Info> _infoPromise;
            struct VideoFrameRequest
            {
                VideoFrameRequest() {}
                VideoFrameRequest(VideoFrameRequest&&) = default;

                otime::RationalTime time = invalidTime;
                std::promise<avio::VideoFrame> promise;
            };
            std::list<VideoFrameRequest> _videoFrameRequests;
            std::condition_variable _requestCV;
            std::mutex _requestMutex;
            otime::RationalTime _currentTime = invalidTime;
            std::list<std::shared_ptr<imaging::Image> > _imageBuffer;

            AVFormatContext* _avFormatContext = nullptr;
            int _avVideoStream = -1;
            std::map<int, AVCodecParameters*> _avCodecParameters;
            std::map<int, AVCodecContext*> _avCodecContext;
            AVFrame* _avFrame = nullptr;
            AVFrame* _avFrame2 = nullptr;
            SwsContext* _swsContext = nullptr;

            std::thread _thread;
            std::atomic<bool> _running;
            std::atomic<bool> _stopped;
        };

        //! FFmpeg writer.
        class Write : public avio::IWrite
        {
        protected:
            void _init(
                const std::string& fileName,
                const avio::Info&,
                const avio::Options&);
            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const std::string& fileName,
                const avio::Info&,
                const avio::Options&);

            void writeVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;

        private:
            void _encodeVideo(AVFrame*);

            AVOutputFormat* _avOutputFormat = nullptr;
            AVFormatContext* _avFormatContext = nullptr;
            AVCodec* _avCodec = nullptr;
            AVStream* _avVideoStream = nullptr;
            AVPacket* _avPacket = nullptr;
            AVFrame* _avFrame = nullptr;
            AVPixelFormat _avPixelFormatIn = AV_PIX_FMT_NONE;
            AVPixelFormat _avPixelFormatOut = AV_PIX_FMT_YUV420P;
            AVFrame* _avFrame2 = nullptr;
            SwsContext* _swsContext = nullptr;
        };

        //! FFmpeg Plugin
        class Plugin : public avio::IPlugin
        {
        protected:
            void _init();
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create();

            std::shared_ptr<avio::IRead> read(
                const std::string& fileName,
                const avio::Options& = avio::Options()) override;
            std::vector<imaging::PixelType> getWritePixelTypes() const override;
            std::shared_ptr<avio::IWrite> write(
                const std::string& fileName,
                const avio::Info&,
                const avio::Options& = avio::Options()) override;
        };
    }

    TLR_ENUM_SERIALIZE(ffmpeg::Profile);
}
