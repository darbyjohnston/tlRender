// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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

        struct Read::Private
        {
            io::Info info;
            std::promise<io::Info> infoPromise;

            bool yuvToRGBConversion = false;
            audio::Info audioConvertInfo;
            size_t threadCount = ffmpeg::threadCount;
            size_t requestTimeout = 5;
            size_t videoBufferSize = 4;
            otime::RationalTime audioBufferSize = otime::RationalTime(1.0, 1.0);

            struct VideoRequest
            {
                otime::RationalTime time = time::invalidTime;
                std::promise<io::VideoData> promise;
            };
            std::list<std::shared_ptr<VideoRequest> > videoRequests;
            std::shared_ptr<VideoRequest> currentVideoRequest;
            otime::RationalTime currentVideoTime = time::invalidTime;

            struct AudioRequest
            {
                otime::TimeRange time = time::invalidTimeRange;
                std::promise<io::AudioData> promise;
            };
            std::list<std::shared_ptr<AudioRequest> > audioRequests;
            std::shared_ptr<AudioRequest> currentAudioRequest;
            otime::RationalTime currentAudioTime = time::invalidTime;

            struct Video
            {
                AVFormatContext* avFormatContext = nullptr;
                AVIOBufferData avIOBufferData;
                uint8_t* avIOContextBuffer = nullptr;
                AVIOContext* avIOContext = nullptr;
                int avStream = -1;
                std::map<int, AVCodecParameters*> avCodecParameters;
                std::map<int, AVCodecContext*> avCodecContext;
                AVFrame* avFrame = nullptr;
                AVFrame* avFrame2 = nullptr;
                AVPixelFormat avInputPixelFormat = AV_PIX_FMT_NONE;
                AVPixelFormat avOutputPixelFormat = AV_PIX_FMT_NONE;
                SwsContext* swsContext = nullptr;
                std::list<std::shared_ptr<imaging::Image> > buffer;
            };
            Video video;

            struct Audio
            {
                AVFormatContext* avFormatContext = nullptr;
                AVIOBufferData avIOBufferData;
                uint8_t* avIOContextBuffer = nullptr;
                AVIOContext* avIOContext = nullptr;
                int avStream = -1;
                std::map<int, AVCodecParameters*> avCodecParameters;
                std::map<int, AVCodecContext*> avCodecContext;
                AVFrame* avFrame = nullptr;
                SwrContext* swrContext = nullptr;
                std::list<std::shared_ptr<audio::Audio> > buffer;
            };
            Audio audio;

            std::condition_variable cv;
            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running;
            bool stopped = false;

            std::chrono::steady_clock::time_point logTimer;

            int decodeVideo();
            void copyVideo(const std::shared_ptr<imaging::Image>&);

            int decodeAudio();
        };
    }
}
