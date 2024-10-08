// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <tlTimeline/Util.h>

#include <tlCore/AudioResample.h>
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
        struct Player::Private
        {
            otime::RationalTime loopPlayback(const otime::RationalTime&, bool& looped);

            void clearRequests();
            void clearCache();
            void cacheUpdate();

            bool hasAudio() const;
            void playbackReset(const otime::RationalTime&);
            void audioInit(const std::shared_ptr<system::Context>&);
            void audioReset(const otime::RationalTime&);
            static size_t getAudioChannelCount(
                const audio::Info& input,
                const audio::Info& output);
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
            otime::TimeRange timeRange = time::invalidTimeRange;
            io::Info ioInfo;

            std::shared_ptr<observer::Value<double> > speed;
            std::shared_ptr<observer::Value<Playback> > playback;
            std::shared_ptr<observer::Value<Loop> > loop;
            std::shared_ptr<observer::Value<otime::RationalTime> > currentTime;
            std::shared_ptr<observer::Value<otime::RationalTime> > seek;
            std::shared_ptr<observer::Value<otime::TimeRange> > inOutRange;
            std::shared_ptr<observer::List<std::shared_ptr<Timeline> > > compare;
            std::shared_ptr<observer::Value<CompareTimeMode> > compareTime;
            std::shared_ptr<observer::Value<io::Options> > ioOptions;
            std::shared_ptr<observer::Value<int> > videoLayer;
            std::shared_ptr<observer::List<int> > compareVideoLayers;
            std::shared_ptr<observer::List<VideoData> > currentVideoData;
            std::shared_ptr<observer::Value<audio::DeviceID> > audioDevice;
            std::shared_ptr<observer::Value<float> > volume;
            std::shared_ptr<observer::Value<bool> > mute;
            std::shared_ptr<observer::Value<double> > audioOffset;
            std::shared_ptr<observer::List<AudioData> > currentAudioData;
            std::shared_ptr<observer::Value<PlayerCacheOptions> > cacheOptions;
            std::shared_ptr<observer::Value<PlayerCacheInfo> > cacheInfo;
            std::shared_ptr<observer::ValueObserver<bool> > timelineObserver;
            std::shared_ptr<observer::ListObserver<audio::DeviceInfo> > audioDevicesObserver;
            std::shared_ptr<observer::ValueObserver<audio::DeviceID> > defaultAudioDeviceObserver;

#if defined(TLRENDER_AUDIO)
            audio::Info audioInfo;
            std::unique_ptr<RtAudio> rtAudio;
#endif // TLRENDER_AUDIO

            std::atomic<bool> running;

            struct Mutex
            {
                Playback playback = Playback::Stop;
                otime::RationalTime currentTime = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                std::vector<std::shared_ptr<Timeline> > compare;
                CompareTimeMode compareTime = CompareTimeMode::Relative;
                io::Options ioOptions;
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                std::vector<VideoData> currentVideoData;
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

            struct Thread
            {
                Playback playback = Playback::Stop;
                otime::RationalTime currentTime = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                std::vector<std::shared_ptr<Timeline> > compare;
                CompareTimeMode compareTime = CompareTimeMode::Relative;
                io::Options ioOptions;
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                double audioOffset = 0.0;
                CacheDirection cacheDirection = CacheDirection::Forward;
                PlayerCacheOptions cacheOptions;
                std::map<otime::RationalTime, std::vector<VideoRequest> > videoDataRequests;
                std::map<otime::RationalTime, std::vector<VideoData> > videoDataCache;
                std::map<int64_t, AudioRequest> audioDataRequests;
                std::chrono::steady_clock::time_point cacheTimer;
                std::chrono::steady_clock::time_point logTimer;
                std::thread thread;
            };
            Thread thread;

            struct AudioMutex
            {
                Playback playback = Playback::Stop;
                double speed = 0.0;
                float volume = 1.F;
                bool mute = false;
                std::chrono::steady_clock::time_point muteTimeout;
                double audioOffset = 0.0;
                std::map<int64_t, AudioData> audioDataCache;
                bool reset = false;
                otime::RationalTime start = time::invalidTime;
                int64_t frame = 0;
                std::mutex mutex;
            };
            AudioMutex audioMutex;

            struct AudioThread
            {
                audio::Info info;
                int64_t inputFrame = 0;
                int64_t outputFrame = 0;
                size_t cacheRetryCount = 0;
                std::shared_ptr<audio::AudioResample> resample;
                std::list<std::shared_ptr<audio::Audio> > buffer;
                std::shared_ptr<audio::Audio> silence;
            };
            AudioThread audioThread;

            struct NoAudio
            {
                std::chrono::steady_clock::time_point playbackTimer;
                otime::RationalTime start = time::invalidTime;
            };
            NoAudio noAudio;
        };
    }
}
