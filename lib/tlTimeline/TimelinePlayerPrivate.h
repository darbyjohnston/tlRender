// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/TimelinePlayer.h>

#include <tlCore/AudioConvert.h>
#include <tlCore/LRUCache.h>

#if defined(TLRENDER_AUDIO)
#include <rtaudio/RtAudio.h>
#endif // TLRENDER_AUDIO

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace timeline
    {
        enum class CacheDirection
        {
            Forward,
            Reverse
        };

        struct TimelinePlayer::Private
        {
            otime::RationalTime loopPlayback(const otime::RationalTime&);

            void cacheUpdate(
                const otime::RationalTime& currentTime,
                const otime::TimeRange& inOutRange,
                uint16_t videoLayer,
                double audioOffset,
                CacheDirection,
                const PlayerCacheOptions&);

            void resetAudioTime();
#if defined(TLRENDER_AUDIO)
            static int rtAudioCallback(
                void* outputBuffer,
                void* inputBuffer,
                unsigned int nFrames,
                double streamTime,
                RtAudioStreamStatus status,
                void* userData);
            static void rtAudioErrorCallback(
                RtAudioError::Type type,
                const std::string& errorText);
#endif // TLRENDER_AUDIO

            void log(const std::shared_ptr<system::Context>&);

            PlayerOptions playerOptions;
            std::shared_ptr<Timeline> timeline;
            io::Info ioInfo;

            std::shared_ptr<observer::Value<double> > speed;
            std::shared_ptr<observer::Value<Playback> > playback;
            std::shared_ptr<observer::Value<Loop> > loop;
            std::shared_ptr<observer::Value<otime::RationalTime> > currentTime;
            std::shared_ptr<observer::Value<otime::TimeRange> > inOutRange;
            std::shared_ptr<observer::Value<uint16_t> > videoLayer;
            std::shared_ptr<observer::Value<VideoData> > currentVideoData;
            std::shared_ptr<observer::Value<float> > volume;
            std::shared_ptr<observer::Value<bool> > mute;
            std::shared_ptr<observer::Value<double> > audioOffset;
            std::shared_ptr<observer::List<AudioData> > currentAudioData;
            std::shared_ptr<observer::Value<PlayerCacheOptions> > cacheOptions;
            std::shared_ptr<observer::Value<PlayerCacheInfo> > cacheInfo;

            struct ExternalTime
            {
                std::shared_ptr<TimelinePlayer> player;
                std::shared_ptr<observer::ValueObserver<Playback> > playbackObserver;
                std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            };
            ExternalTime externalTime;

            struct Mutex
            {
                Playback playback = Playback::Stop;
                otime::RationalTime playbackStartTime = time::invalidTime;
                std::chrono::steady_clock::time_point playbackStartTimer;
                otime::RationalTime currentTime = time::invalidTime;
                bool externalTime = false;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                uint16_t videoLayer = 0;
                VideoData currentVideoData;
                double audioOffset = 0.0;
                std::vector<AudioData> currentAudioData;
                bool clearRequests = false;
                bool clearCache = false;
                CacheDirection cacheDirection = CacheDirection::Forward;
                PlayerCacheOptions cacheOptions;
                PlayerCacheInfo cacheInfo;
                std::mutex mutex;
            };
            Mutex mutex;

            struct AudioMutex
            {
                double speed = 0.0;
                float volume = 1.F;
                bool mute = false;
                std::chrono::steady_clock::time_point muteTimeout;
                std::map<int64_t, AudioData> audioDataCache;
                bool reset = false;
                std::mutex mutex;
            };
            AudioMutex audioMutex;

            struct Thread
            {
                std::map<otime::RationalTime, std::future<VideoData> > videoDataRequests;
                std::map<otime::RationalTime, VideoData> videoDataCache;
#if defined(TLRENDER_AUDIO)
                std::unique_ptr<RtAudio> rtAudio;
#endif // TLRENDER_AUDIO
                std::map<int64_t, std::future<AudioData> > audioDataRequests;
                std::chrono::steady_clock::time_point cacheTimer;
                std::chrono::steady_clock::time_point logTimer;
                std::atomic<bool> running;
                std::thread thread;
            };
            Thread thread;

            struct AudioThread
            {
                audio::Info info;
                std::shared_ptr<audio::AudioConvert> convert;
                std::list<std::shared_ptr<audio::Audio> > buffer;
                size_t rtAudioCurrentFrame = 0;
            };
            AudioThread audioThread;
        };
    }
}
