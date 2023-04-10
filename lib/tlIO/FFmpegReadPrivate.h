// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/FFmpeg.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>

} // extern "C"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace tl
{
    namespace ffmpeg
    {
        struct AVIOBufferData
        {
            AVIOBufferData();
            AVIOBufferData(const uint8_t* p, size_t size);

            const uint8_t* p = nullptr;
            const uint8_t* pCurrent = nullptr;
            size_t size = 0;
        };

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize);
        int64_t avIOBufferSeek(void* opaque, int64_t offset, int whence);

        const size_t avIOContextBufferSize = 4096;

        struct Options
        {
            bool yuvToRGBConversion = false;
            audio::Info audioConvertInfo;
            size_t threadCount = ffmpeg::threadCount;
            size_t requestTimeout = 5;
            size_t videoBufferSize = 4;
            otime::RationalTime audioBufferSize = otime::RationalTime(2.0, 1.0);
        };

        class ReadVideo
        {
        public:
            ReadVideo(
                const std::string& fileName,
                const std::vector<file::MemoryRead>& memory,
                const Options& options);

            ~ReadVideo();

            bool isValid() const;
            const imaging::Info& getInfo() const;
            const otime::TimeRange& getTimeRange() const;
            const imaging::Tags& getTags() const;

            void start();
            void seek(const otime::RationalTime&);
            void process(const otime::RationalTime& currentTime);

            bool isBufferEmpty() const;
            std::shared_ptr<imaging::Image> popBuffer();

            bool isEOF() const;

        private:
            int _decode(const otime::RationalTime& currentTime);
            void _copy(const std::shared_ptr<imaging::Image>&);

            std::string _fileName;
            Options _options;
            imaging::Info _info;
            otime::TimeRange _timeRange = time::invalidTimeRange;
            imaging::Tags _tags;

            AVFormatContext* _avFormatContext = nullptr;
            AVIOBufferData _avIOBufferData;
            uint8_t* _avIOContextBuffer = nullptr;
            AVIOContext* _avIOContext = nullptr;
            AVRational _avSpeed = { 24, 1 };
            int _avStream = -1;
            std::map<int, AVCodecParameters*> _avCodecParameters;
            std::map<int, AVCodecContext*> _avCodecContext;
            AVFrame* _avFrame = nullptr;
            AVFrame* _avFrame2 = nullptr;
            AVPixelFormat _avInputPixelFormat = AV_PIX_FMT_NONE;
            AVPixelFormat _avOutputPixelFormat = AV_PIX_FMT_NONE;
            SwsContext* _swsContext = nullptr;
            std::list<std::shared_ptr<imaging::Image> > _buffer;
            bool _eof = false;
        };

        class ReadAudio
        {
        public:
            ReadAudio(
                const std::string& fileName,
                const std::vector<file::MemoryRead>&,
                double videoRate,
                const Options&);

            ~ReadAudio();

            bool isValid() const;
            const audio::Info& getInfo() const;
            const otime::TimeRange& getTimeRange() const;
            const imaging::Tags& getTags() const;

            void start();
            void seek(const otime::RationalTime&);
            void process(const otime::RationalTime& currentTime);

            size_t getBufferSize() const;
            void bufferCopy(uint8_t*, size_t byteCount);

            bool isEOF() const;

        private:
            int _decode(const otime::RationalTime& currentTime);

            std::string _fileName;
            Options _options;
            audio::Info _info;
            otime::TimeRange _timeRange = time::invalidTimeRange;
            imaging::Tags _tags;

            AVFormatContext* _avFormatContext = nullptr;
            AVIOBufferData _avIOBufferData;
            uint8_t* _avIOContextBuffer = nullptr;
            AVIOContext* _avIOContext = nullptr;
            int _avStream = -1;
            std::map<int, AVCodecParameters*> _avCodecParameters;
            std::map<int, AVCodecContext*> _avCodecContext;
            AVFrame* _avFrame = nullptr;
            SwrContext* _swrContext = nullptr;
            std::list<std::shared_ptr<audio::Audio> > _buffer;
            bool _eof = false;
        };

        struct Read::Private
        {
            Options options;

            std::shared_ptr<ReadVideo> readVideo;
            std::shared_ptr<ReadAudio> readAudio;

            io::Info info;
            struct InfoRequest
            {
                std::promise<io::Info> promise;
            };

            struct VideoRequest
            {
                otime::RationalTime time = time::invalidTime;
                std::promise<io::VideoData> promise;
            };
            struct VideoMutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::shared_ptr<VideoRequest> videoRequest;
                bool stopped = false;
                std::mutex mutex;
            };
            VideoMutex videoMutex;
            struct VideoThread
            {
                otime::RationalTime currentTime = time::invalidTime;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            VideoThread videoThread;

            struct AudioRequest
            {
                otime::TimeRange time = time::invalidTimeRange;
                std::promise<io::AudioData> promise;
            };
            struct AudioMutex
            {
                std::list<std::shared_ptr<AudioRequest> > requests;
                std::shared_ptr<AudioRequest> currentRequest;
                bool stopped = false;
                std::mutex mutex;
            };
            AudioMutex audioMutex;
            struct AudioThread
            {
                otime::RationalTime currentTime = time::invalidTime;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            AudioThread audioThread;
        };
    }
}
