// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/TimelinePlayer.h>

#include <tlrCore/Assert.h>
#include <tlrCore/AudioSystem.h>
#include <tlrCore/Error.h>
#include <tlrCore/File.h>
#include <tlrCore/LogSystem.h>
#include <tlrCore/Math.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>
#include <tlrCore/TimelineUtil.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/stackAlgorithm.h>
#include <opentimelineio/timeline.h>

#if defined(TLR_ENABLE_PYTHON)
#include <Python.h>
#endif

#include <array>
#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tlr
{
    namespace timeline
    {
        TLR_ENUM_IMPL(TimerMode, "System", "Audio");
        TLR_ENUM_SERIALIZE_IMPL(TimerMode);

        TLR_ENUM_IMPL(AudioBufferFrameCount, "16", "32", "64", "128", "256", "512", "1024");
        TLR_ENUM_SERIALIZE_IMPL(AudioBufferFrameCount);

        size_t getAudioBufferFrameCount(AudioBufferFrameCount value)
        {
            const std::array<size_t, static_cast<size_t>(AudioBufferFrameCount::Count)> data =
            {
                16,
                32,
                64,
                128,
                256,
                512,
                1024
            };
            return data[static_cast<size_t>(value)];
        }

        TLR_ENUM_IMPL(Playback, "Stop", "Forward", "Reverse");
        TLR_ENUM_SERIALIZE_IMPL(Playback);

        TLR_ENUM_IMPL(Loop, "Loop", "Once", "Ping-Pong");
        TLR_ENUM_SERIALIZE_IMPL(Loop);

        TLR_ENUM_IMPL(TimeAction,
            "Start",
            "End",
            "FramePrev",
            "FramePrevX10",
            "FramePrevX100",
            "FrameNext",
            "FrameNextX10",
            "FrameNextX100");
        TLR_ENUM_SERIALIZE_IMPL(TimeAction);

        otime::RationalTime loop(
            const otime::RationalTime& value,
            const otime::TimeRange& range,
            bool* looped)
        {
            auto out = value;
            if (out < range.start_time())
            {
                if (looped)
                {
                    *looped = true;
                }
                out = range.end_time_inclusive();
            }
            else if (out > range.end_time_inclusive())
            {
                if (looped)
                {
                    *looped = true;
                }
                out = range.start_time();
            }
            return out;
        }

        std::vector<otime::TimeRange> loop(
            const otime::TimeRange& value,
            const otime::TimeRange& range)
        {
            std::vector<otime::TimeRange> out;
            if (value.duration() >= range.duration())
            {
                out.push_back(range);
            }
            else if (value.start_time() >= range.start_time() &&
                value.end_time_inclusive() <= range.end_time_inclusive())
            {
                out.push_back(value);
            }
            else if (value.start_time() < range.start_time())
            {
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    range.end_time_exclusive() - (range.start_time() - value.start_time()),
                    range.end_time_inclusive()));
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    range.start_time(),
                    value.end_time_inclusive()));
            }
            else if (value.end_time_inclusive() > range.end_time_inclusive())
            {
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    value.start_time(),
                    range.end_time_inclusive()));
                out.push_back(otime::TimeRange::range_from_start_end_time_inclusive(
                    range.start_time(),
                    range.start_time() + (value.end_time_inclusive() - range.end_time_exclusive())));
            }
            return out;
        }

        namespace
        {
            enum class CacheDirection
            {
                Forward,
                Reverse
            };
        }

        struct TimelinePlayer::Private
        {
            otime::RationalTime loopPlayback(const otime::RationalTime&);

            void cacheUpdate(
                const otime::RationalTime& currentTime,
                const otime::TimeRange& inOutRange,
                uint16_t videoLayer,
                double audioOffset,
                CacheDirection,
                const otime::RationalTime& cacheReadAhead,
                const otime::RationalTime& cacheReadBehind);

            void resetAudioTime();
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

            void log(const std::shared_ptr<core::Context>&);

            PlayerOptions playerOptions;
            std::shared_ptr<Timeline> timeline;
            avio::Info avInfo;

            std::shared_ptr<observer::Value<double> > speed;
            std::shared_ptr<observer::Value<Playback> > playback;
            std::shared_ptr<observer::Value<Loop> > loop;
            std::shared_ptr<observer::Value<otime::RationalTime> > currentTime;
            std::shared_ptr<observer::Value<otime::TimeRange> > inOutRange;
            std::shared_ptr<observer::Value<uint16_t> > videoLayer;
            std::shared_ptr<observer::Value<VideoData> > video;
            std::shared_ptr<observer::Value<float> > volume;
            std::shared_ptr<observer::Value<bool> > mute;
            std::shared_ptr<observer::Value<double> > audioOffset;
            std::shared_ptr<observer::Value<float> > cachePercentage;
            std::shared_ptr<observer::List<otime::TimeRange> > cachedVideoFrames;
            std::shared_ptr<observer::List<otime::TimeRange> > cachedAudioFrames;

            struct ExternalTime
            {
                std::shared_ptr<TimelinePlayer> player;
                std::shared_ptr<observer::ValueObserver<Playback> > playbackObserver;
                std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            };
            ExternalTime externalTime;

            struct MutexData
            {
                Playback playback = Playback::Stop;
                otime::RationalTime playbackStartTime = time::invalidTime;
                std::chrono::steady_clock::time_point playbackStartTimer;
                otime::RationalTime currentTime = time::invalidTime;
                bool externalTime = false;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                uint16_t videoLayer = 0;
                VideoData videoData;
                double audioOffset = 0.0;
                bool clearRequests = false;
                std::vector<otime::TimeRange> cachedVideoFrames;
                std::vector<otime::TimeRange> cachedAudioFrames;
                bool clearCache = false;
                CacheDirection cacheDirection = CacheDirection::Forward;
                otime::RationalTime cacheReadAhead = otime::RationalTime(4.0, 1.0);
                otime::RationalTime cacheReadBehind = otime::RationalTime(0.4, 1.0);
            };
            MutexData mutexData;
            std::mutex mutex;

            struct AudioMutexData
            {
                double speed = 0.0;
                float volume = 1.F;
                bool mute = false;
                std::chrono::steady_clock::time_point muteTimeout;
                std::map<int64_t, AudioData> audioDataCache;
                size_t rtAudioCurrentFrame = 0;
            };
            AudioMutexData audioMutexData;
            std::mutex audioMutex;

            struct ThreadData
            {
                std::map<otime::RationalTime, std::future<VideoData> > videoDataRequests;
                std::map<otime::RationalTime, VideoData> videoDataCache;
                std::unique_ptr<RtAudio> rtAudio;
                std::map<int64_t, std::future<AudioData> > audioDataRequests;
                std::atomic<bool> running;
            };
            ThreadData threadData;
            std::thread thread;

            std::chrono::steady_clock::time_point logTimer;
        };

        void TimelinePlayer::_init(
            const std::shared_ptr<Timeline>& timeline,
            const std::shared_ptr<core::Context>& context,
            const PlayerOptions& playerOptions)
        {
            TLR_PRIVATE_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(string::Format("    Timer mode: {0}").
                    arg(playerOptions.timerMode));
                lines.push_back(string::Format("    Audio buffer frame count: {0}").
                    arg(playerOptions.audioBufferFrameCount));
                lines.push_back(string::Format("    Mute timeout: {0}ms").
                    arg(playerOptions.muteTimeout.count()));
                lines.push_back(string::Format("    Sleep timeout: {0}ms").
                    arg(playerOptions.sleepTimeout.count()));
                logSystem->print(
                    string::Format("tlr::timeline::TimelinePlayer {0}").arg(this),
                    string::join(lines, "\n"));
            }

            p.playerOptions = playerOptions;
            p.timeline = timeline;
            p.avInfo = p.timeline->getAVInfo();

            // Create observers.
            p.speed = observer::Value<double>::create(p.timeline->getDuration().rate());
            p.playback = observer::Value<Playback>::create(Playback::Stop);
            p.loop = observer::Value<Loop>::create(Loop::Loop);
            p.currentTime = observer::Value<otime::RationalTime>::create(p.timeline->getGlobalStartTime());
            p.inOutRange = observer::Value<otime::TimeRange>::create(
                otime::TimeRange(p.timeline->getGlobalStartTime(), p.timeline->getDuration()));
            p.videoLayer = observer::Value<uint16_t>::create();
            p.video = observer::Value<VideoData>::create();
            p.volume = observer::Value<float>::create(1.F);
            p.mute = observer::Value<bool>::create(false);
            p.audioOffset = observer::Value<double>::create(0.0);
            p.cachePercentage = observer::Value<float>::create();
            p.cachedVideoFrames = observer::List<otime::TimeRange>::create();
            p.cachedAudioFrames = observer::List<otime::TimeRange>::create();

            // Create a new thread.
            p.mutexData.currentTime = p.currentTime->get();
            p.mutexData.inOutRange = p.inOutRange->get();
            p.mutexData.audioOffset = p.audioOffset->get();
            p.audioMutexData.speed = p.speed->get();
            p.threadData.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLR_PRIVATE_P();

                    if (auto context = getContext().lock())
                    {
                        // Initialize audio.
                        auto audioSystem = context->getSystem<audio::System>();
                        if (!audioSystem->getDevices().empty() &&
                            p.avInfo.audio.channelCount > 0 &&
                            p.avInfo.audio.dataType != audio::DataType::None &&
                            p.avInfo.audio.sampleRate > 0)
                        {
                            try
                            {
                                p.threadData.rtAudio.reset(new RtAudio);
                                RtAudio::StreamParameters rtParameters;
                                auto audioSystem = context->getSystem<audio::System>();
                                rtParameters.deviceId = audioSystem->getDefaultOutputDevice();
                                rtParameters.nChannels = p.avInfo.audio.channelCount;
                                unsigned int rtBufferFrames = getAudioBufferFrameCount(p.playerOptions.audioBufferFrameCount);
                                p.threadData.rtAudio->openStream(
                                    &rtParameters,
                                    nullptr,
                                    audio::toRtAudio(p.avInfo.audio.dataType),
                                    p.avInfo.audio.sampleRate,
                                    &rtBufferFrames,
                                    p.rtAudioCallback,
                                    _p.get(),
                                    nullptr,
                                    p.rtAudioErrorCallback);
                                p.threadData.rtAudio->startStream();
                            }
                            catch (const std::exception& e)
                            {
                                std::stringstream ss;
                                ss << "Cannot open audio stream: " << e.what();
                                context->log("tlr::core::TimelinePlayer", ss.str(), core::LogType::Error);
                            }
                        }
                    }

                    p.logTimer = std::chrono::steady_clock::now();

                    while (p.threadData.running)
                    {
                        // Get mutex protected values.
                        Playback playback = Playback::Stop;
                        otime::RationalTime currentTime = time::invalidTime;
                        otime::TimeRange inOutRange = time::invalidTimeRange;
                        uint16_t videoLayer = 0;
                        double audioOffset = 0.0;
                        bool clearRequests = false;
                        bool clearCache = false;
                        CacheDirection cacheDirection = CacheDirection::Forward;
                        otime::RationalTime cacheReadAhead;
                        otime::RationalTime cacheReadBehind;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            playback = p.mutexData.playback;
                            currentTime = p.mutexData.currentTime;
                            inOutRange = p.mutexData.inOutRange;
                            videoLayer = p.mutexData.videoLayer;
                            audioOffset = p.mutexData.audioOffset;
                            clearRequests = p.mutexData.clearRequests;
                            p.mutexData.clearRequests = false;
                            clearCache = p.mutexData.clearCache;
                            p.mutexData.clearCache = false;
                            cacheDirection = p.mutexData.cacheDirection;
                            cacheReadAhead = p.mutexData.cacheReadAhead;
                            cacheReadBehind = p.mutexData.cacheReadBehind;
                        }

                        // Clear requests.
                        if (clearRequests)
                        {
                            p.timeline->cancelRequests();
                            p.threadData.videoDataRequests.clear();
                            p.threadData.audioDataRequests.clear();
                        }

                        // Clear the cache.
                        if (clearCache)
                        {
                            p.threadData.videoDataCache.clear();
                            {
                                std::unique_lock<std::mutex> lock(p.mutex);
                                p.mutexData.cachedVideoFrames.clear();
                                p.mutexData.cachedAudioFrames.clear();
                            }
                            {
                                std::unique_lock<std::mutex> lock(p.audioMutex);
                                p.audioMutexData.audioDataCache.clear();
                            }
                        }

                        // Update the cache.
                        p.cacheUpdate(
                            currentTime,
                            inOutRange,
                            videoLayer,
                            audioOffset,
                            cacheDirection,
                            cacheReadAhead,
                            cacheReadBehind);

                        // Update the video data.
                        if (!p.avInfo.video.empty())
                        {
                            const auto i = p.threadData.videoDataCache.find(currentTime);
                            if (i != p.threadData.videoDataCache.end())
                            {
                                std::unique_lock<std::mutex> lock(p.mutex);
                                p.mutexData.videoData = i->second;
                            }
                            else
                            {
                                {
                                    std::unique_lock<std::mutex> lock(p.mutex);
                                    p.mutexData.playbackStartTime = p.currentTime->get();
                                    p.mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                                }
                                p.resetAudioTime();
                                {
                                    const auto now = std::chrono::steady_clock::now();
                                    std::unique_lock<std::mutex> lock(p.audioMutex);
                                    p.audioMutexData.muteTimeout = now + p.playerOptions.muteTimeout;
                                }
                            }
                        }

                        // Logging.
                        const auto now = std::chrono::steady_clock::now();
                        const std::chrono::duration<double> diff = now - p.logTimer;
                        if (diff.count() > 10.0)
                        {
                            p.logTimer = now;
                            if (auto context = getContext().lock())
                            {
                                p.log(context);
                            }
                        }

                        // Sleep for a bit...
                        time::sleep(p.playerOptions.sleepTimeout);
                    }
                });
        }

        TimelinePlayer::TimelinePlayer() :
            _p(new Private)
        {}

        TimelinePlayer::~TimelinePlayer()
        {
            TLR_PRIVATE_P();
            if (p.threadData.rtAudio && p.threadData.rtAudio->isStreamOpen())
            {
                try
                {
                    p.threadData.rtAudio->abortStream();
                }
                catch (const std::exception&)
                {}
            }
            p.threadData.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<TimelinePlayer> TimelinePlayer::create(
            const std::shared_ptr<Timeline>& timeline,
            const std::shared_ptr<core::Context>& context,
            const PlayerOptions& playerOptions)
        {
            auto out = std::shared_ptr<TimelinePlayer>(new TimelinePlayer);
            out->_init(timeline, context, playerOptions);
            return out;
        }

        const std::weak_ptr<core::Context>& TimelinePlayer::getContext() const
        {
            return _p->timeline->getContext();
        }
        
        const std::shared_ptr<Timeline>& TimelinePlayer::getTimeline() const
        {
            return _p->timeline;
        }

        const file::Path& TimelinePlayer::getPath() const
        {
            return _p->timeline->getPath();
        }

        const file::Path& TimelinePlayer::getAudioPath() const
        {
            return _p->timeline->getAudioPath();
        }

        const PlayerOptions& TimelinePlayer::getPlayerOptions() const
        {
            return _p->playerOptions;
        }

        const Options& TimelinePlayer::getOptions() const
        {
            return _p->timeline->getOptions();
        }

        const otime::RationalTime& TimelinePlayer::getGlobalStartTime() const
        {
            return _p->timeline->getGlobalStartTime();
        }

        const otime::RationalTime& TimelinePlayer::getDuration() const
        {
            return _p->timeline->getDuration();
        }

        const avio::Info& TimelinePlayer::getAVInfo() const
        {
            return _p->avInfo;
        }

        double TimelinePlayer::getDefaultSpeed() const
        {
            return _p->timeline->getDuration().rate();
        }

        std::shared_ptr<observer::IValue<double> > TimelinePlayer::observeSpeed() const
        {
            return _p->speed;
        }

        void TimelinePlayer::setSpeed(double value)
        {
            TLR_PRIVATE_P();
            if (p.speed->setIfChanged(value))
            {
                if (p.playback->get() != Playback::Stop)
                {
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.mutexData.playbackStartTime = p.currentTime->get();
                        p.mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                    }
                    p.resetAudioTime();
                }
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex);
                    p.audioMutexData.speed = value;
                }
            }
        }

        std::shared_ptr<observer::IValue<Playback> > TimelinePlayer::observePlayback() const
        {
            return _p->playback;
        }

        void TimelinePlayer::setPlayback(Playback value)
        {
            TLR_PRIVATE_P();
            switch (p.loop->get())
            {
            case Loop::Once:
                switch (value)
                {
                case Playback::Forward:
                    if (p.currentTime->get() == p.inOutRange->get().end_time_inclusive())
                    {
                        seek(p.inOutRange->get().start_time());
                    }
                    break;
                case Playback::Reverse:
                    if (p.currentTime->get() == p.inOutRange->get().start_time())
                    {
                        seek(p.inOutRange->get().end_time_inclusive());
                    }
                    break;
                default: break;
                }
                break;
            case Loop::PingPong:
                switch (value)
                {
                case Playback::Forward:
                    if (p.currentTime->get() == p.inOutRange->get().end_time_inclusive())
                    {
                        value = Playback::Reverse;
                    }
                    break;
                case Playback::Reverse:
                    if (p.currentTime->get() == p.inOutRange->get().start_time())
                    {
                        value = Playback::Forward;
                    }
                    break;
                default: break;
                }
                break;
            default: break;
            }
            if (p.playback->setIfChanged(value))
            {
                if (value != Playback::Stop)
                {
                    {
                        std::unique_lock<std::mutex> lock(p.mutex);
                        p.mutexData.playback = value;
                        p.mutexData.playbackStartTime = p.currentTime->get();
                        p.mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                        p.mutexData.cacheDirection = Playback::Forward == value ? CacheDirection::Forward : CacheDirection::Reverse;
                    }
                    p.resetAudioTime();
                }
                else
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.mutexData.playback = value;
                    p.mutexData.clearRequests = true;
                }
            }
        }

        std::shared_ptr<observer::IValue<Loop> > TimelinePlayer::observeLoop() const
        {
            return _p->loop;
        }

        void TimelinePlayer::setLoop(Loop value)
        {
            _p->loop->setIfChanged(value);
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> > TimelinePlayer::observeCurrentTime() const
        {
            return _p->currentTime;
        }

        void TimelinePlayer::seek(const otime::RationalTime& time)
        {
            TLR_PRIVATE_P();

            // Loop the time.
            auto range = otime::TimeRange(p.timeline->getGlobalStartTime(), p.timeline->getDuration());
            const auto tmp = loop(time, range);

            if (p.currentTime->setIfChanged(tmp))
            {
                //std::cout << "seek: " << tmp << std::endl;

                // Update playback.
                if (p.playback->get() != Playback::Stop)
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.mutexData.playbackStartTime = tmp;
                    p.mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                }

                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    p.mutexData.currentTime = tmp;
                    p.mutexData.clearRequests = true;
                }
                p.resetAudioTime();
            }
        }

        void TimelinePlayer::timeAction(TimeAction time)
        {
            TLR_PRIVATE_P();
            setPlayback(timeline::Playback::Stop);
            const auto& duration = p.timeline->getDuration();
            const auto& currentTime = p.currentTime->get();
            switch (time)
            {
            case TimeAction::Start:
                seek(p.inOutRange->get().start_time());
                break;
            case TimeAction::End:
                seek(p.inOutRange->get().end_time_inclusive());
                break;
            case TimeAction::FramePrev:
                seek(currentTime - otime::RationalTime(1, duration.rate()));
                break;
            case TimeAction::FramePrevX10:
                seek(currentTime - otime::RationalTime(10, duration.rate()));
                break;
            case TimeAction::FramePrevX100:
                seek(currentTime - otime::RationalTime(100, duration.rate()));
                break;
            case TimeAction::FrameNext:
                seek(currentTime + otime::RationalTime(1, duration.rate()));
                break;
            case TimeAction::FrameNextX10:
                seek(currentTime + otime::RationalTime(10, duration.rate()));
                break;
            case TimeAction::FrameNextX100:
                seek(currentTime + otime::RationalTime(100, duration.rate()));
                break;
            default: break;
            }
        }

        void TimelinePlayer::start()
        {
            timeAction(TimeAction::Start);
        }

        void TimelinePlayer::end()
        {
            timeAction(TimeAction::End);
        }

        void TimelinePlayer::framePrev()
        {
            timeAction(TimeAction::FramePrev);
        }

        void TimelinePlayer::frameNext()
        {
            timeAction(TimeAction::FrameNext);
        }

        void TimelinePlayer::setExternalTime(const std::shared_ptr<TimelinePlayer>& value)
        {
            TLR_PRIVATE_P();
            if (value == p.externalTime.player)
                return;
            p.externalTime.player = value;
            if (p.externalTime.player)
            {
                auto weak = std::weak_ptr<TimelinePlayer>(shared_from_this());
                p.externalTime.playbackObserver = observer::ValueObserver<Playback>::create(
                    p.externalTime.player->observePlayback(),
                    [weak](Playback value)
                    {
                        if (auto player = weak.lock())
                        {
                            player->setPlayback(value);
                        }
                    });
                p.externalTime.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                    p.externalTime.player->observeCurrentTime(),
                    [weak](const otime::RationalTime& value)
                    {
                        if (auto player = weak.lock())
                        {
                            player->_p->currentTime->setIfChanged(time::floor(value.rescaled_to(player->getDuration().rate())));
                        }
                    });
            }
            else
            {
                p.externalTime.playbackObserver.reset();
                p.externalTime.currentTimeObserver.reset();
            }
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.mutexData.externalTime = p.externalTime.player.get();
            }
        }

        std::shared_ptr<observer::IValue<otime::TimeRange> > TimelinePlayer::observeInOutRange() const
        {
            return _p->inOutRange;
        }

        void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
        {
            TLR_PRIVATE_P();
            if (p.inOutRange->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.mutexData.inOutRange = value;
                p.mutexData.clearRequests = true;
            }
        }

        void TimelinePlayer::setInPoint()
        {
            TLR_PRIVATE_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time(
                p.currentTime->get(),
                p.inOutRange->get().end_time_exclusive()));
        }

        void TimelinePlayer::resetInPoint()
        {
            TLR_PRIVATE_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time(
                p.timeline->getGlobalStartTime(),
                p.inOutRange->get().end_time_exclusive()));
        }

        void TimelinePlayer::setOutPoint()
        {
            TLR_PRIVATE_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                p.inOutRange->get().start_time(),
                p.currentTime->get()));
        }

        void TimelinePlayer::resetOutPoint()
        {
            TLR_PRIVATE_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time(
                p.inOutRange->get().start_time(),
                p.timeline->getGlobalStartTime() + p.timeline->getDuration()));
        }

        std::shared_ptr<observer::IValue<uint16_t> > TimelinePlayer::observeVideoLayer() const
        {
            return _p->videoLayer;
        }

        void TimelinePlayer::setVideoLayer(uint16_t layer)
        {
            TLR_PRIVATE_P();
            if (p.videoLayer->setIfChanged(layer))
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.mutexData.videoLayer = layer;
                p.mutexData.clearRequests = true;
                p.mutexData.clearCache = true;
            }
        }

        std::shared_ptr<observer::IValue<VideoData> > TimelinePlayer::observeVideo() const
        {
            return _p->video;
        }

        std::shared_ptr<observer::IValue<float> > TimelinePlayer::observeVolume() const
        {
            return _p->volume;
        }

        void TimelinePlayer::setVolume(float value)
        {
            TLR_PRIVATE_P();
            if (p.volume->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex);
                p.audioMutexData.volume = value;
            }
        }

        std::shared_ptr<observer::IValue<bool> > TimelinePlayer::observeMute() const
        {
            return _p->mute;
        }

        void TimelinePlayer::setMute(bool value)
        {
            TLR_PRIVATE_P();
            if (p.mute->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex);
                p.audioMutexData.mute = value;
            }
        }

        std::shared_ptr<observer::IValue<double> > TimelinePlayer::observeAudioOffset() const
        {
            return _p->audioOffset;
        }

        void TimelinePlayer::setAudioOffset(double value)
        {
            TLR_PRIVATE_P();
            if (p.audioOffset->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.mutexData.audioOffset = value;
            }
        }

        otime::RationalTime TimelinePlayer::getCacheReadAhead()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.mutexData.cacheReadAhead;
        }

        otime::RationalTime TimelinePlayer::getCacheReadBehind()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            return p.mutexData.cacheReadBehind;
        }

        void TimelinePlayer::setCacheReadAhead(const otime::RationalTime& value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.mutexData.cacheReadAhead = value;
        }

        void TimelinePlayer::setCacheReadBehind(const otime::RationalTime& value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.mutex);
            p.mutexData.cacheReadBehind = value;
        }

        std::shared_ptr<observer::IValue<float> > TimelinePlayer::observeCachePercentage() const
        {
            return _p->cachePercentage;
        }

        std::shared_ptr<observer::IList<otime::TimeRange> > TimelinePlayer::observeCachedVideoFrames() const
        {
            return _p->cachedVideoFrames;
        }

        std::shared_ptr<observer::IList<otime::TimeRange> > TimelinePlayer::observeCachedAudioFrames() const
        {
            return _p->cachedAudioFrames;
        }

        void TimelinePlayer::tick()
        {
            TLR_PRIVATE_P();

            // Calculate the current time.
            const auto& duration = p.timeline->getDuration();
            const auto playback = p.playback->get();
            if (playback != Playback::Stop && !p.externalTime.player)
            {
                const double timelineSpeed = p.timeline->getDuration().rate();
                const double speed = p.speed->get();

                otime::RationalTime playbackStartTime = time::invalidTime;
                std::chrono::steady_clock::time_point playbackStartTimer;
                {
                    std::unique_lock<std::mutex> lock(p.mutex);
                    playbackStartTime = p.mutexData.playbackStartTime;
                    playbackStartTimer = p.mutexData.playbackStartTimer;
                }
                double seconds = 0.0;
                if (p.threadData.rtAudio &&
                    p.threadData.rtAudio->isStreamRunning() &&
                    TimerMode::Audio == p.playerOptions.timerMode &&
                    math::fuzzyCompare(timelineSpeed, speed))
                {
                    seconds = p.threadData.rtAudio->getStreamTime();
                }
                else
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<double> diff = now - playbackStartTimer;
                    seconds = diff.count() * (speed / timelineSpeed);
                }
                if (Playback::Reverse == playback)
                {
                    seconds = -seconds;
                }
                const otime::RationalTime currentTime = p.loopPlayback(
                    playbackStartTime + time::floor(otime::RationalTime(seconds, 1.0).rescaled_to(duration.rate())));
                //const double currentTimeDiff = abs(currentTime.value() - p.currentTime->get().value());
                if (p.currentTime->setIfChanged(currentTime))
                {
                    //std::cout << "current time: " << p.currentTime->get() << " / " << currentTimeDiff << std::endl;
                }
            }

            // Sync with the thread.
            VideoData videoData;
            otime::RationalTime cacheReadAhead;
            otime::RationalTime cacheReadBehind;
            std::vector<otime::TimeRange> cachedVideoFrames;
            std::vector<otime::TimeRange> cachedAudioFrames;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                p.mutexData.currentTime = p.currentTime->get();
                videoData = p.mutexData.videoData;
                cacheReadAhead = p.mutexData.cacheReadAhead;
                cacheReadBehind = p.mutexData.cacheReadBehind;
                cachedVideoFrames = p.mutexData.cachedVideoFrames;
                cachedAudioFrames = p.mutexData.cachedAudioFrames;
            }
            p.video->setIfChanged(videoData);
            size_t cachedVideoFramesCount = 0;
            for (const auto& i : cachedVideoFrames)
            {
                cachedVideoFramesCount += i.duration().value();
            }
            p.cachePercentage->setIfChanged(
                cachedVideoFramesCount /
                static_cast<float>(cacheReadAhead.rescaled_to(duration.rate()).value() +
                    cacheReadBehind.rescaled_to(duration.rate()).value()) *
                100.F);
            p.cachedVideoFrames->setIfChanged(cachedVideoFrames);
            p.cachedAudioFrames->setIfChanged(cachedAudioFrames);
        }

        otime::RationalTime TimelinePlayer::Private::loopPlayback(const otime::RationalTime& time)
        {
            otime::RationalTime out = time;

            const auto& range = inOutRange->get();
            switch (loop->get())
            {
            case Loop::Loop:
            {
                bool looped = false;
                out = timeline::loop(out, range, &looped);
                if (looped)
                {
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playbackStartTime = out;
                        mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                    }
                    resetAudioTime();
                }
                break;
            }
            case Loop::Once:
                if (out < range.start_time())
                {
                    out = range.start_time();
                    playback->setIfChanged(Playback::Stop);
                }
                else if (out > range.end_time_inclusive())
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Stop);
                }
                break;
            case Loop::PingPong:
            {
                const auto playbackValue = playback->get();
                if (out < range.start_time() && Playback::Reverse == playbackValue)
                {
                    out = range.start_time();
                    playback->setIfChanged(Playback::Forward);
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playbackStartTime = out;
                        mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                    }
                    resetAudioTime();
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Reverse);
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        mutexData.playbackStartTime = out;
                        mutexData.playbackStartTimer = std::chrono::steady_clock::now();
                    }
                    resetAudioTime();
                }
                break;
            }
            default: break;
            }

            return out;
        }

        void TimelinePlayer::Private::cacheUpdate(
            const otime::RationalTime& currentTime,
            const otime::TimeRange& inOutRange,
            uint16_t videoLayer,
            double audioOffset,
            CacheDirection cacheDirection,
            const otime::RationalTime& cacheReadAhead,
            const otime::RationalTime& cacheReadBehind)
        {
            // Get the ranges to be cached.
            const otime::RationalTime& globalStartTime = timeline->getGlobalStartTime();
            const otime::RationalTime& duration = timeline->getDuration();
            const otime::RationalTime audioOffsetRescaled =
                time::floor(otime::RationalTime(audioOffset, 1.0).rescaled_to(duration.rate()));
            const otime::RationalTime audioOffsetAhead =
                audioOffsetRescaled.value() < 0.0 ? -audioOffsetRescaled : otime::RationalTime(0.0, duration.rate());
            const otime::RationalTime audioOffsetBehind =
                audioOffsetRescaled.value() > 0.0 ? audioOffsetRescaled : otime::RationalTime(0.0, duration.rate());
            const otime::RationalTime cacheReadAheadRescaled =
                time::floor(cacheReadAhead.rescaled_to(duration.rate()));
            const otime::RationalTime cacheReadBehindRescaled =
                time::floor(cacheReadBehind.rescaled_to(duration.rate()));
            otime::TimeRange range = time::invalidTimeRange;
            switch (cacheDirection)
            {
            case CacheDirection::Forward:
                range = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - cacheReadBehindRescaled - audioOffsetBehind,
                    currentTime + cacheReadAheadRescaled + audioOffsetAhead);
                break;
            case CacheDirection::Reverse:
                range = otime::TimeRange::range_from_start_end_time_inclusive(
                    currentTime - cacheReadAheadRescaled - audioOffsetAhead,
                    currentTime + cacheReadBehindRescaled + audioOffsetBehind);
                break;
            default: break;
            }
            const otime::TimeRange inOutAudioOffsetRange = otime::TimeRange::range_from_start_end_time_inclusive(
                std::max(
                    inOutRange.start_time() - audioOffsetBehind,
                    globalStartTime),
                std::min(
                    inOutRange.end_time_inclusive() + audioOffsetAhead,
                    globalStartTime + duration - otime::RationalTime(1.0, duration.rate())));
            const auto ranges = timeline::loop(range, inOutAudioOffsetRange);
            timeline->setActiveRanges(ranges);

            // Remove old video from the cache.
            auto videoDataCacheIt = threadData.videoDataCache.begin();
            while (videoDataCacheIt != threadData.videoDataCache.end())
            {
                bool old = true;
                for (const auto& i : ranges)
                {
                    if (i.contains(videoDataCacheIt->second.time))
                    {
                        old = false;
                        break;
                    }
                }
                if (old)
                {
                    videoDataCacheIt = threadData.videoDataCache.erase(videoDataCacheIt);
                    continue;
                }
                ++videoDataCacheIt;
            }

            // Remove old audio from the cache.
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                auto audioDataCacheIt = audioMutexData.audioDataCache.begin();
                while (audioDataCacheIt != audioMutexData.audioDataCache.end())
                {
                    bool old = true;
                    for (const auto& i : ranges)
                    {
                        if (i.intersects(otime::TimeRange(
                            otime::RationalTime(audioDataCacheIt->second.seconds, 1.0),
                            otime::RationalTime(1.0, 1.0))))
                        {
                            old = false;
                            break;
                        }
                    }
                    if (old)
                    {
                        //std::cout << "audio remove: " << audioDataCacheIt->second.seconds << std::endl;
                        audioDataCacheIt = audioMutexData.audioDataCache.erase(audioDataCacheIt);
                        continue;
                    }
                    ++audioDataCacheIt;
                }
            }

            // Get uncached video.
            if (!avInfo.video.empty())
            {
                for (const auto& i : ranges)
                {
                    for (otime::RationalTime time = i.start_time();
                        time < i.end_time_exclusive();
                        time += otime::RationalTime(1.0, duration.rate()))
                    {
                        const auto j = threadData.videoDataCache.find(time);
                        if (j == threadData.videoDataCache.end())
                        {
                            const auto k = threadData.videoDataRequests.find(time);
                            if (k == threadData.videoDataRequests.end())
                            {
                                threadData.videoDataRequests[time] = timeline->getVideo(time, videoLayer);
                            }
                        }
                    }
                }
            }

            // Get uncached audio.
            if (avInfo.audio.isValid())
            {
                std::vector<otime::TimeRange> audioCacheRanges;
                //std::vector<std::string> s;
                for (const auto& i : ranges)
                {
                    const otime::TimeRange range(
                        time::floor(i.start_time().rescaled_to(1.0)),
                        time::ceil(i.duration().rescaled_to(1.0)));
                    //std::stringstream ss;
                    //ss << range.start_time().value() << "/" << range.duration().value();
                    //s.push_back(ss.str());
                    audioCacheRanges.push_back(range);
                }
                //std::cout << "audio cache: " << string::join(s, ", ") << std::endl;
                std::unique_lock<std::mutex> lock(audioMutex);
                for (const auto& i : audioCacheRanges)
                {
                    for (auto j = i.start_time(); j <= i.end_time_inclusive(); j += otime::RationalTime(1.0, 1.0))
                    {
                        const int64_t time = j.value();
                        const auto k = audioMutexData.audioDataCache.find(time);
                        if (k == audioMutexData.audioDataCache.end())
                        {
                            const auto l = threadData.audioDataRequests.find(time);
                            if (l == threadData.audioDataRequests.end())
                            {
                                //std::cout << "audio request: " << time << std::endl;
                                threadData.audioDataRequests[time] = timeline->getAudio(time);
                            }
                        }
                    }
                }
            }

            // Check for finished video.
            auto videoDataRequestsIt = threadData.videoDataRequests.begin();
            while (videoDataRequestsIt != threadData.videoDataRequests.end())
            {
                if (videoDataRequestsIt->second.valid() &&
                    videoDataRequestsIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto data = videoDataRequestsIt->second.get();
                    data.time = videoDataRequestsIt->first;
                    threadData.videoDataCache[data.time] = data;
                    videoDataRequestsIt = threadData.videoDataRequests.erase(videoDataRequestsIt);
                    continue;
                }
                ++videoDataRequestsIt;
            }

            // Check for finished audio.
            auto audioDataRequestsIt = threadData.audioDataRequests.begin();
            while (audioDataRequestsIt != threadData.audioDataRequests.end())
            {
                if (audioDataRequestsIt->second.valid() &&
                    audioDataRequestsIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto data = audioDataRequestsIt->second.get();
                    //std::cout << "audio result: " << data.seconds << std::endl;
                    data.seconds = audioDataRequestsIt->first;
                    {
                        std::unique_lock<std::mutex> lock(audioMutex);
                        audioMutexData.audioDataCache[data.seconds] = data;
                    }
                    audioDataRequestsIt = threadData.audioDataRequests.erase(audioDataRequestsIt);
                    continue;
                }
                ++audioDataRequestsIt;
            }

            // Update cached frames.
            std::vector<otime::RationalTime> cachedVideoFrames;
            for (const auto& i : threadData.videoDataCache)
            {
                cachedVideoFrames.push_back(i.second.time);
            }
            std::vector<otime::RationalTime> cachedAudioFrames;
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                for (const auto& i : audioMutexData.audioDataCache)
                {
                    cachedAudioFrames.push_back(otime::RationalTime(i.second.seconds, 1.0));
                }
            }
            auto cachedVideoRanges = toRanges(cachedVideoFrames);
            auto cachedAudioRanges = toRanges(cachedAudioFrames);
            for (auto& i : cachedAudioRanges)
            {
                i = otime::TimeRange(i.start_time().rescaled_to(duration.rate()), i.duration().rescaled_to(duration.rate()));
            }
            {
                std::unique_lock<std::mutex> lock(mutex);
                mutexData.cachedVideoFrames = cachedVideoRanges;
                mutexData.cachedAudioFrames = cachedAudioRanges;
            }
        }

        void TimelinePlayer::Private::resetAudioTime()
        {
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                audioMutexData.rtAudioCurrentFrame = 0;
            }
            if (threadData.rtAudio &&
                threadData.rtAudio->isStreamRunning())
            {
                try
                {
                    threadData.rtAudio->setStreamTime(0.0);
                }
                catch (const std::exception&)
                {}
            }
        }

        int TimelinePlayer::Private::rtAudioCallback(
            void* outputBuffer,
            void* inputBuffer,
            unsigned int nFrames,
            double streamTime,
            RtAudioStreamStatus status,
            void* userData)
        {
            auto p = reinterpret_cast<TimelinePlayer::Private*>(userData);
            
            // Get mutex protected values.
            Playback playback = Playback::Stop;
            double playbackStartTimeInSeconds = 0.0;
            bool externalTime = false;
            {
                std::unique_lock<std::mutex> lock(p->mutex);
                playback = p->mutexData.playback;
                playbackStartTimeInSeconds =
                    p->mutexData.playbackStartTime.rescaled_to(1.0).value() -
                    p->mutexData.audioOffset;
                externalTime = p->mutexData.externalTime;
            }
            double speed = 0.F;
            float volume = 1.F;
            bool mute = false;
            std::chrono::steady_clock::time_point muteTimeout;
            size_t rtAudioCurrentFrame = 0;
            {
                std::unique_lock<std::mutex> lock(p->audioMutex);
                speed = p->audioMutexData.speed;
                volume = p->audioMutexData.volume;
                mute = p->audioMutexData.mute;
                muteTimeout = p->audioMutexData.muteTimeout;
                rtAudioCurrentFrame = p->audioMutexData.rtAudioCurrentFrame;
            }

            // Audio information constants.
            const uint8_t channelCount = p->avInfo.audio.channelCount;
            const audio::DataType dataType = p->avInfo.audio.dataType;
            const size_t byteCount = p->avInfo.audio.getByteCount();

            // Zero output audio data.
            std::memset(outputBuffer, 0, nFrames * byteCount);

            switch (playback)
            {
            case Playback::Forward:
            {
                // Time in seconds for indexing into the audio cache.
                int64_t seconds = playbackStartTimeInSeconds +
                    rtAudioCurrentFrame / static_cast<double>(p->avInfo.audio.sampleRate);

                // Offset into the audio data.
                int64_t offset = playbackStartTimeInSeconds * p->avInfo.audio.sampleRate +
                    rtAudioCurrentFrame -
                    seconds * p->avInfo.audio.sampleRate;

                const auto now = std::chrono::steady_clock::now();

                // Copy audio data to RtAudio.
                if (speed == p->timeline->getDuration().rate() &&
                    !externalTime &&
                    !mute &&
                    now >= muteTimeout &&
                    offset >= 0)
                {
                    size_t sampleCount = nFrames;
                    uint8_t* outputBufferP = reinterpret_cast<uint8_t*>(outputBuffer);
                    //size_t count = 0;
                    while (sampleCount > 0)
                    {
                        // Get audio data from the cache.
                        std::vector<std::shared_ptr<audio::Audio> > audioData;
                        {
                            std::unique_lock<std::mutex> lock(p->audioMutex);
                            const auto j = p->audioMutexData.audioDataCache.find(seconds);
                            if (j != p->audioMutexData.audioDataCache.end())
                            {
                                for (const auto& layer : j->second.layers)
                                {
                                    audioData.push_back(layer.audio);
                                }
                            }
                        }

                        size_t size = sampleCount;
                        if (!audioData.empty())
                        {
                            // Get pointers to the audio data. Only audio data
                            // that has the same information (channels, data
                            // type, sample rate) is used.
                            std::vector<const uint8_t*> audioDataP;
                            for (size_t j = 0; j < audioData.size(); ++j)
                            {
                                if (j > 0 ? audioData[j]->getInfo() == audioData[0]->getInfo() : true)
                                {
                                    audioDataP.push_back(audioData[j]->getData() + offset * byteCount);
                                }
                            }

                            size = std::min(size, static_cast<size_t>(audioData[0]->getSampleCount() - offset));
                            
                            //std::cout << count <<
                            //    " samples: " << sampleCount <<
                            //    " seconds: " << seconds <<
                            //    " frame: " << rtAudioCurrentFrame <<
                            //    " offset: " << offset <<
                            //    " size: " << size << std::endl;
                            //std::memcpy(outputBufferP, audioData[0]->getData() + offset * byteCount, size * byteCount);
                            
                            audio::mix(
                                audioDataP.data(),
                                audioDataP.size(),
                                outputBufferP,
                                volume,
                                size,
                                channelCount,
                                dataType);
                        }

                        offset = 0;
                        sampleCount -= size;
                        ++seconds;
                        outputBufferP += size * byteCount;
                        //++count;
                    }
                }

                // Update the audio frame.
                {
                    std::unique_lock<std::mutex> lock(p->audioMutex);
                    p->audioMutexData.rtAudioCurrentFrame += nFrames;
                }

                break;
            }
            case Playback::Reverse:
                // Update the audio frame.
                {
                    std::unique_lock<std::mutex> lock(p->audioMutex);
                    p->audioMutexData.rtAudioCurrentFrame += nFrames;
                }
                break;
            default: break;
            }

            return 0;
        }

        void TimelinePlayer::Private::rtAudioErrorCallback(
            RtAudioError::Type type,
            const std::string& errorText)
        {}

        void TimelinePlayer::Private::log(const std::shared_ptr<core::Context>& context)
        {
            const std::string id = string::Format("tlr::timeline::TimelinePlayer {0}").arg(this);

            // Get mutex protected values.
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            uint16_t videoLayer = 0;
            std::vector<otime::TimeRange> cachedVideoFrames;
            std::vector<otime::TimeRange> cachedAudioFrames;
            otime::RationalTime cacheReadAhead = time::invalidTime;
            otime::RationalTime cacheReadBehind = time::invalidTime;
            {
                std::unique_lock<std::mutex> lock(mutex);
                currentTime = mutexData.currentTime;
                inOutRange = mutexData.inOutRange;
                videoLayer = mutexData.videoLayer;
                cachedVideoFrames = mutexData.cachedVideoFrames;
                cachedAudioFrames = mutexData.cachedAudioFrames;
                cacheReadAhead = mutexData.cacheReadAhead;
                cacheReadBehind = mutexData.cacheReadBehind;
            }
            size_t audioDataCacheSize = 0;
            {
                std::unique_lock<std::mutex> lock(audioMutex);
                audioDataCacheSize = audioMutexData.audioDataCache.size();
            }

            // Create an array of characters to draw the timeline.
            const size_t lineLength = 80;
            std::string currentTimeDisplay(lineLength, '.');
            double n = (currentTime - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
            currentTimeDisplay[math::clamp(n, 0.0, 1.0) * (lineLength - 1)] = 'T';

            // Create an array of characters to draw the cached video frames.
            std::string cachedVideoFramesDisplay(lineLength, '.');
            for (const auto& i : cachedVideoFrames)
            {
                double n = (i.start_time() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t0 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t1 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                for (size_t j = t0; j <= t1; ++j)
                {
                    cachedVideoFramesDisplay[j] = 'V';
                }
            }

            // Create an array of characters to draw the cached audio frames.
            std::string cachedAudioFramesDisplay(lineLength, '.');
            for (const auto& i : cachedAudioFrames)
            {
                double n = (i.start_time() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t0 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                n = (i.end_time_inclusive() - timeline->getGlobalStartTime()).value() / timeline->getDuration().value();
                const size_t t1 = math::clamp(n, 0.0, 1.0) * (lineLength - 1);
                for (size_t j = t0; j <= t1; ++j)
                {
                    cachedAudioFramesDisplay[j] = 'A';
                }
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(id, string::Format(
                "\n"
                "    Path: {0}\n"
                "    Current time: {1}\n"
                "    In/out range: {2}\n"
                "    Video layer: {3}\n"
                "    Cache: {4} read ahead, {5} read behind\n"
                "    Video: {6} requests, {7} cached\n"
                "    Audio: {8} requests, {9} cached\n"
                "    {10}\n"
                "    {11}\n"
                "    {12}\n"
                "    (T=current time, V=cached video, A=cached audio)").
                arg(timeline->getPath().get()).
                arg(currentTime).
                arg(inOutRange).
                arg(videoLayer).
                arg(cacheReadAhead).
                arg(cacheReadBehind).
                arg(threadData.videoDataRequests.size()).
                arg(threadData.videoDataCache.size()).
                arg(threadData.audioDataRequests.size()).
                arg(audioDataCacheSize).
                arg(currentTimeDisplay).
                arg(cachedVideoFramesDisplay).
                arg(cachedAudioFramesDisplay));
        }
    }
}
