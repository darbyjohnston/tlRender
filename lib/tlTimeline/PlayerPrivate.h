// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <tlTimeline/Util.h>

#include <tlCore/AudioResample.h>
#include <tlCore/LRUCache.h>

#if defined(TLRENDER_SDL2)
#include <SDL2/SDL.h>
#endif // TLRENDER_SDL2
#if defined(TLRENDER_SDL3)
#include <SDL3/SDL.h>
#endif // TLRENDER_SDL3

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace timeline
    {
        struct Player::Private
        {
            OTIO_NS::RationalTime loopPlayback(const OTIO_NS::RationalTime&, bool& looped);

            void clearRequests();
            void clearCache();
            void cacheUpdate();

            bool hasAudio() const;
            void playbackReset(const OTIO_NS::RationalTime&);
            void audioInit(const std::shared_ptr<system::Context>&);
            void audioReset(const OTIO_NS::RationalTime&);
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
            void sdlCallback(uint8_t* stream, int len);
#if defined(TLRENDER_SDL2)
            static void sdl2Callback(void* user, Uint8* stream, int len);
#elif defined(TLRENDER_SDL3)
            static void sdl3Callback(void* user, SDL_AudioStream *stream, int additional_amount, int total_amount);
#endif // TLRENDER_SDL2
#endif // TLRENDER_SDL2

            void log(const std::shared_ptr<system::Context>&);

            PlayerOptions playerOptions;
            std::shared_ptr<Timeline> timeline;
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            io::Info ioInfo;

            std::shared_ptr<observer::Value<double> > speed;
            std::shared_ptr<observer::Value<Playback> > playback;
            std::shared_ptr<observer::Value<Loop> > loop;
            std::shared_ptr<observer::Value<OTIO_NS::RationalTime> > currentTime;
            std::shared_ptr<observer::Value<OTIO_NS::RationalTime> > seek;
            std::shared_ptr<observer::Value<OTIO_NS::TimeRange> > inOutRange;
            std::shared_ptr<observer::List<std::shared_ptr<Timeline> > > compare;
            std::shared_ptr<observer::Value<CompareTimeMode> > compareTime;
            std::shared_ptr<observer::Value<io::Options> > ioOptions;
            std::shared_ptr<observer::Value<int> > videoLayer;
            std::shared_ptr<observer::List<int> > compareVideoLayers;
            std::shared_ptr<observer::List<VideoData> > currentVideoData;
            std::shared_ptr<observer::Value<audio::DeviceID> > audioDevice;
            std::shared_ptr<observer::Value<float> > volume;
            std::shared_ptr<observer::Value<bool> > mute;
            std::shared_ptr<observer::List<bool> > channelMute;
            std::shared_ptr<observer::Value<double> > audioOffset;
            std::shared_ptr<observer::List<AudioData> > currentAudioData;
            std::shared_ptr<observer::Value<PlayerCacheOptions> > cacheOptions;
            std::shared_ptr<observer::Value<PlayerCacheInfo> > cacheInfo;
            std::shared_ptr<observer::ValueObserver<bool> > timelineObserver;
            std::shared_ptr<observer::ListObserver<audio::DeviceInfo> > audioDevicesObserver;
            std::shared_ptr<observer::ValueObserver<audio::DeviceInfo> > defaultAudioDeviceObserver;

            bool audioDevices = false;
            audio::Info audioInfo;
#if defined(TLRENDER_SDL2)
            int sdlID = 0;
#elif defined(TLRENDER_SDL3)
            SDL_AudioStream* sdlStream = nullptr;
#endif // TLRENDER_SDL2

            std::atomic<bool> running;

            struct Mutex
            {
                Playback playback = Playback::Stop;
                OTIO_NS::RationalTime currentTime = time::invalidTime;
                OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
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
                OTIO_NS::RationalTime currentTime = time::invalidTime;
                OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
                std::vector<std::shared_ptr<Timeline> > compare;
                CompareTimeMode compareTime = CompareTimeMode::Relative;
                io::Options ioOptions;
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                double audioOffset = 0.0;
                CacheDirection cacheDirection = CacheDirection::Forward;
                PlayerCacheOptions cacheOptions;
                std::map<OTIO_NS::RationalTime, std::vector<VideoRequest> > videoDataRequests;
                std::map<OTIO_NS::RationalTime, std::vector<VideoData> > videoDataCache;
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
                std::vector<bool> channelMute;
                std::chrono::steady_clock::time_point muteTimeout;
                double audioOffset = 0.0;
                std::map<int64_t, AudioData> audioDataCache;
                bool reset = false;
                OTIO_NS::RationalTime start = time::invalidTime;
                int64_t frame = 0;
                std::mutex mutex;
            };
            AudioMutex audioMutex;

            struct AudioThread
            {
                audio::Info info;
                int64_t inputFrame = 0;
                int64_t outputFrame = 0;
                std::shared_ptr<audio::AudioResample> resample;
                std::list<std::shared_ptr<audio::Audio> > buffer;
                std::shared_ptr<audio::Audio> silence;
            };
            AudioThread audioThread;

            struct NoAudio
            {
                std::chrono::steady_clock::time_point playbackTimer;
                OTIO_NS::RationalTime start = time::invalidTime;
            };
            NoAudio noAudio;
        };
    }
}
