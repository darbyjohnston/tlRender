// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <tlTimeline/Util.h>

#include <tlCore/AudioResample.h>

#include <dtk/core/LRUCache.h>

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
            void audioInit(const std::shared_ptr<dtk::Context>&);
            void audioReset(const OTIO_NS::RationalTime&);
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
            void sdlCallback(uint8_t* stream, int len);
#if defined(TLRENDER_SDL2)
            static void sdl2Callback(void* user, Uint8* stream, int len);
#elif defined(TLRENDER_SDL3)
            static void sdl3Callback(void* user, SDL_AudioStream *stream, int additional_amount, int total_amount);
#endif // TLRENDER_SDL2
#endif // TLRENDER_SDL2

            void log(const std::shared_ptr<dtk::Context>&);

            PlayerOptions playerOptions;
            std::shared_ptr<Timeline> timeline;
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            io::Info ioInfo;

            std::shared_ptr<dtk::ObservableValue<double> > speed;
            std::shared_ptr<dtk::ObservableValue<Playback> > playback;
            std::shared_ptr<dtk::ObservableValue<Loop> > loop;
            std::shared_ptr<dtk::ObservableValue<OTIO_NS::RationalTime> > currentTime;
            std::shared_ptr<dtk::ObservableValue<OTIO_NS::RationalTime> > seek;
            std::shared_ptr<dtk::ObservableValue<OTIO_NS::TimeRange> > inOutRange;
            std::shared_ptr<dtk::ObservableList<std::shared_ptr<Timeline> > > compare;
            std::shared_ptr<dtk::ObservableValue<CompareTime> > compareTime;
            std::shared_ptr<dtk::ObservableValue<io::Options> > ioOptions;
            std::shared_ptr<dtk::ObservableValue<int> > videoLayer;
            std::shared_ptr<dtk::ObservableList<int> > compareVideoLayers;
            std::shared_ptr<dtk::ObservableList<VideoData> > currentVideoData;
            std::shared_ptr<dtk::ObservableValue<audio::DeviceID> > audioDevice;
            std::shared_ptr<dtk::ObservableValue<float> > volume;
            std::shared_ptr<dtk::ObservableValue<bool> > mute;
            std::shared_ptr<dtk::ObservableList<bool> > channelMute;
            std::shared_ptr<dtk::ObservableValue<double> > audioOffset;
            std::shared_ptr<dtk::ObservableList<AudioData> > currentAudioData;
            std::shared_ptr<dtk::ObservableValue<PlayerCacheOptions> > cacheOptions;
            std::shared_ptr<dtk::ObservableValue<PlayerCacheInfo> > cacheInfo;
            std::shared_ptr<dtk::ValueObserver<bool> > timelineObserver;
            std::shared_ptr<dtk::ListObserver<audio::DeviceInfo> > audioDevicesObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceInfo> > defaultAudioDeviceObserver;

            bool audioDevices = false;
            audio::Info audioInfo;
#if defined(TLRENDER_SDL2)
            int sdlID = 0;
#elif defined(TLRENDER_SDL3)
            SDL_AudioStream* sdlStream = nullptr;
#endif // TLRENDER_SDL2

            std::atomic<bool> running;

            struct PlaybackState
            {
                Playback playback = Playback::Stop;
                OTIO_NS::RationalTime currentTime = time::invalidTime;
                OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
                std::vector<std::shared_ptr<Timeline> > compare;
                CompareTime compareTime = CompareTime::Relative;
                io::Options ioOptions;
                int videoLayer = 0;
                std::vector<int> compareVideoLayers;
                double audioOffset = 0.0;
                PlayerCacheOptions cacheOptions;

                bool operator == (const PlaybackState&) const;
                bool operator != (const PlaybackState&) const;
            };

            struct Mutex
            {
                PlaybackState state;
                bool clearRequests = false;
                bool clearCache = false;
                CacheDirection cacheDirection = CacheDirection::Forward;
                std::vector<VideoData> currentVideoData;
                std::vector<AudioData> currentAudioData;
                PlayerCacheInfo cacheInfo;
                std::mutex mutex;
            };
            Mutex mutex;

            struct Thread
            {
                OTIO_NS::RationalTime getVideoTime(int64_t) const;
                int64_t getAudioTime(int64_t) const;

                PlaybackState state;
                CacheDirection cacheDirection = CacheDirection::Forward;

                struct VideoRequestData
                {
                    size_t byteCount = 0;
                    std::vector<VideoRequest> list;
                };
                std::map<OTIO_NS::RationalTime, VideoRequestData> videoDataRequests;
                dtk::LRUCache<OTIO_NS::RationalTime, std::vector<VideoData> > videoCache;
                int64_t videoFillFrame = 0;
                size_t videoFillByteCount = 0;

                struct AudioRequestData
                {
                    size_t byteCount = 0;
                    AudioRequest request;
                };
                std::map<int64_t, AudioRequestData> audioDataRequests;
                int64_t audioFillSeconds = 0;
                size_t audioFillByteCount = 0;

                std::chrono::steady_clock::time_point cacheTimer;
                std::chrono::steady_clock::time_point logTimer;
                std::thread thread;
            };
            Thread thread;

            struct AudioState
            {
                Playback playback = Playback::Stop;
                double speed = 0.0;
                float volume = 1.F;
                bool mute = false;
                std::vector<bool> channelMute;
                std::chrono::steady_clock::time_point muteTimeout;
                double audioOffset = 0.0;

                bool operator == (const AudioState&) const;
                bool operator != (const AudioState&) const;
            };

            struct AudioMutex
            {
                AudioState state;
                dtk::LRUCache<int64_t, AudioData> cache;
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
