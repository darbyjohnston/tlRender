// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <dtk/core/Context.h>
#include <dtk/core/Error.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>
#include <dtk/core/Time.h>

namespace tl
{
    namespace timeline
    {
        bool PlayerCacheInfo::operator == (const PlayerCacheInfo& other) const
        {
            return
                videoPercentage == other.videoPercentage &&
                audioPercentage == other.audioPercentage &&
                video == other.video &&
                audio == other.audio;
        }

        bool PlayerCacheInfo::operator != (const PlayerCacheInfo& other) const
        {
            return !(*this == other);
        }

        DTK_ENUM_IMPL(
            Playback,
            "Stop",
            "Forward",
            "Reverse");

        DTK_ENUM_IMPL(
            Loop,
            "Loop",
            "Once",
            "Ping-Pong");

        DTK_ENUM_IMPL(TimeAction,
            "Start",
            "End",
            "FramePrev",
            "FramePrevX10",
            "FramePrevX100",
            "FrameNext",
            "FrameNextX10",
            "FrameNextX100",
            "JumpBack1s",
            "JumpBack10s",
            "JumpForward1s",
            "JumpForward10s");

        void Player::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<Timeline>& timeline,
            const PlayerOptions& playerOptions)
        {
            DTK_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(dtk::Format("    Video cache: {0}GB").
                    arg(playerOptions.cache.videoGB));
                lines.push_back(dtk::Format("    Audio cache: {0}GB").
                    arg(playerOptions.cache.audioGB));
                lines.push_back(dtk::Format("    Cache read behind: {0}").
                    arg(playerOptions.cache.readBehind));
                lines.push_back(dtk::Format("    Audio buffer frame count: {0}").
                    arg(playerOptions.audioBufferFrameCount));
                lines.push_back(dtk::Format("    Mute timeout: {0}ms").
                    arg(playerOptions.muteTimeout.count()));
                lines.push_back(dtk::Format("    Sleep timeout: {0}ms").
                    arg(playerOptions.sleepTimeout.count()));
                logSystem->print(
                    dtk::Format("tl::timeline::Player {0}").arg(this),
                    dtk::join(lines, "\n"));
            }

            p.playerOptions = playerOptions;
            p.timeline = timeline;
            p.timeRange = timeline->getTimeRange();
            p.ioInfo = timeline->getIOInfo();

            // Create observers.
            p.speed = dtk::ObservableValue<double>::create(p.timeRange.duration().rate());
            p.playback = dtk::ObservableValue<Playback>::create(Playback::Stop);
            p.loop = dtk::ObservableValue<Loop>::create(Loop::Loop);
            p.currentTime = dtk::ObservableValue<OTIO_NS::RationalTime>::create(
                playerOptions.currentTime != time::invalidTime ?
                playerOptions.currentTime :
                p.timeRange.start_time());
            p.seek = dtk::ObservableValue<OTIO_NS::RationalTime>::create(p.currentTime->get());
            p.inOutRange = dtk::ObservableValue<OTIO_NS::TimeRange>::create(p.timeRange);
            p.compare = dtk::ObservableList<std::shared_ptr<Timeline> >::create();
            p.compareTime = dtk::ObservableValue<CompareTime>::create(CompareTime::Relative);
            p.ioOptions = dtk::ObservableValue<io::Options>::create();
            p.videoLayer = dtk::ObservableValue<int>::create(0);
            p.compareVideoLayers = dtk::ObservableList<int>::create();
            p.currentVideoData = dtk::ObservableList<VideoData>::create();
            p.audioDevice = dtk::ObservableValue<audio::DeviceID>::create(playerOptions.audioDevice);
            p.volume = dtk::ObservableValue<float>::create(1.F);
            p.mute = dtk::ObservableValue<bool>::create(false);
            p.channelMute = dtk::ObservableList<bool>::create();
            p.audioOffset = dtk::ObservableValue<double>::create(0.0);
            p.currentAudioData = dtk::ObservableList<AudioData>::create();
            p.cacheOptions = dtk::ObservableValue<PlayerCacheOptions>::create(playerOptions.cache);
            p.cacheInfo = dtk::ObservableValue<PlayerCacheInfo>::create();
            auto weak = std::weak_ptr<Player>(shared_from_this());
            p.timelineObserver = dtk::ValueObserver<bool>::create(
                p.timeline->observeTimelineChanges(),
                [weak](bool)
                {
                    if (auto player = weak.lock())
                    {
                        player->clearCache();
                    }
                });
            auto audioSystem = context->getSystem<audio::System>();
            p.audioDevicesObserver = dtk::ListObserver<audio::DeviceInfo>::create(
                audioSystem->observeDevices(),
                [weak](const std::vector<audio::DeviceInfo>&)
                {
                    if (auto player = weak.lock())
                    {
                        if (auto context = player->getContext())
                        {
                            player->_p->audioInit(context);
                        }
                    }
                });
            p.defaultAudioDeviceObserver = dtk::ValueObserver<audio::DeviceInfo>::create(
                audioSystem->observeDefaultDevice(),
                [weak](const audio::DeviceInfo&)
                {
                    if (auto player = weak.lock())
                    {
                        if (audio::DeviceID() == player->_p->audioDevice->get())
                        {
                            if (auto context = player->getContext())
                            {
                                player->_p->audioInit(context);
                            }
                        }
                    }
                });

            // Initialize the audio.
            p.audioInit(context);

            // Create a new thread.
            p.mutex.state.currentTime = p.currentTime->get();
            p.mutex.state.inOutRange = p.inOutRange->get();
            p.mutex.state.audioOffset = p.audioOffset->get();
            p.mutex.state.cacheOptions = p.cacheOptions->get();
            p.audioMutex.state.speed = p.speed->get();
            p.log(context);
            p.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    _thread();
                });
        }

        Player::Player() :
            _p(new Private)
        {}

        Player::~Player()
        {
            DTK_P();
            p.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
#if defined(TLRENDER_SDL2)
            if (p.sdlID > 0)
            {
                SDL_CloseAudioDevice(p.sdlID);
                p.sdlID = 0;
            }
#elif defined(TLRENDER_SDL3)
            if (p.sdlStream)
            {
                SDL_DestroyAudioStream(p.sdlStream);
                p.sdlStream = nullptr;
            }
#endif // TLRENDER_SDL2
        }

        std::shared_ptr<Player> Player::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<Timeline>& timeline,
            const PlayerOptions& playerOptions)
        {
            auto out = std::shared_ptr<Player>(new Player);
            out->_init(context, timeline, playerOptions);
            return out;
        }

        std::shared_ptr<dtk::Context> Player::getContext() const
        {
            return _p->timeline->getContext();
        }
        
        const std::shared_ptr<Timeline>& Player::getTimeline() const
        {
            return _p->timeline;
        }

        const file::Path& Player::getPath() const
        {
            return _p->timeline->getPath();
        }

        const file::Path& Player::getAudioPath() const
        {
            return _p->timeline->getAudioPath();
        }

        const PlayerOptions& Player::getPlayerOptions() const
        {
            return _p->playerOptions;
        }

        const Options& Player::getOptions() const
        {
            return _p->timeline->getOptions();
        }

        const OTIO_NS::TimeRange& Player::getTimeRange() const
        {
            return _p->timeRange;
        }

        const io::Info& Player::getIOInfo() const
        {
            return _p->ioInfo;
        }

        double Player::getDefaultSpeed() const
        {
            return _p->timeRange.duration().rate();
        }

        double Player::getSpeed() const
        {
            return _p->speed->get();
        }

        std::shared_ptr<dtk::IObservableValue<double> > Player::observeSpeed() const
        {
            return _p->speed;
        }

        void Player::setSpeed(double value)
        {
            DTK_P();
            if (p.speed->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioMutex.state.speed = value;
                    p.audioReset(p.currentTime->get());
                }
                if (!p.hasAudio())
                {
                    p.playbackReset(p.currentTime->get());
                }
            }
        }

        Playback Player::getPlayback() const
        {
            return _p->playback->get();
        }

        std::shared_ptr<dtk::IObservableValue<Playback> > Player::observePlayback() const
        {
            return _p->playback;
        }

        void Player::setPlayback(Playback value)
        {
            DTK_P();

            // Update the frame for loop modes.
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
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.state.playback = value;
                        p.mutex.state.currentTime = p.currentTime->get();
                        p.mutex.clearRequests = true;
                        p.mutex.cacheDirection = Playback::Forward == value ?
                            CacheDirection::Forward :
                            CacheDirection::Reverse;
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.state.playback = value;
                        p.audioReset(p.currentTime->get());
                    }
                    if (!p.hasAudio())
                    {
                        p.playbackReset(p.currentTime->get());
                    }
                }
                else
                {
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.state.playback = value;
                        p.mutex.clearRequests = true;
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.state.playback = value;
                    }
                }
            }
        }

        bool Player::isStopped() const
        {
            return Playback::Stop == _p->playback->get();
        }

        void Player::stop()
        {
            setPlayback(Playback::Stop);
        }

        void Player::forward()
        {
            setPlayback(Playback::Forward);
        }

        void Player::reverse()
        {
            setPlayback(Playback::Reverse);
        }

        Loop Player::getLoop() const
        {
            return _p->loop->get();
        }

        std::shared_ptr<dtk::IObservableValue<Loop> > Player::observeLoop() const
        {
            return _p->loop;
        }

        void Player::setLoop(Loop value)
        {
            _p->loop->setIfChanged(value);
        }

        const OTIO_NS::RationalTime& Player::getCurrentTime() const
        {
            return _p->currentTime->get();
        }

        std::shared_ptr<dtk::IObservableValue<OTIO_NS::RationalTime> > Player::observeCurrentTime() const
        {
            return _p->currentTime;
        }

        std::shared_ptr<dtk::IObservableValue<OTIO_NS::RationalTime> > Player::observeSeek() const
        {
            return _p->seek;
        }

        void Player::seek(const OTIO_NS::RationalTime& time)
        {
            DTK_P();

            // Loop the time.
            const auto tmp = loop(
                time.rescaled_to(p.timeRange.duration()).floor(),
                p.timeRange);

            if (p.currentTime->setIfChanged(tmp))
            {
                //std::cout << "seek: " << tmp << std::endl;
                p.seek->setAlways(tmp);
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.state.currentTime = tmp;
                    p.mutex.clearRequests = true;
                }
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioReset(tmp);
                }
                if (!p.hasAudio())
                {
                    p.playbackReset(tmp);
                }
            }
        }

        void Player::timeAction(TimeAction time)
        {
            DTK_P();
            const auto& currentTime = p.currentTime->get();
            switch (time)
            {
            case TimeAction::Start:
                setPlayback(timeline::Playback::Stop);
                seek(p.inOutRange->get().start_time());
                break;
            case TimeAction::End:
                setPlayback(timeline::Playback::Stop);
                seek(p.inOutRange->get().end_time_inclusive());
                break;
            case TimeAction::FramePrev:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime - OTIO_NS::RationalTime(1, p.timeRange.duration().rate()));
                break;
            case TimeAction::FramePrevX10:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime - OTIO_NS::RationalTime(10, p.timeRange.duration().rate()));
                break;
            case TimeAction::FramePrevX100:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime - OTIO_NS::RationalTime(100, p.timeRange.duration().rate()));
                break;
            case TimeAction::FrameNext:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime + OTIO_NS::RationalTime(1, p.timeRange.duration().rate()));
                break;
            case TimeAction::FrameNextX10:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime + OTIO_NS::RationalTime(10, p.timeRange.duration().rate()));
                break;
            case TimeAction::FrameNextX100:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime + OTIO_NS::RationalTime(100, p.timeRange.duration().rate()));
                break;
            case TimeAction::JumpBack1s:
                seek(currentTime - OTIO_NS::RationalTime(1.0, 1.0));
                break;
            case TimeAction::JumpBack10s:
                seek(currentTime - OTIO_NS::RationalTime(10.0, 1.0));
                break;
            case TimeAction::JumpForward1s:
                seek(currentTime + OTIO_NS::RationalTime(1.0, 1.0));
                break;
            case TimeAction::JumpForward10s:
                seek(currentTime + OTIO_NS::RationalTime(10.0, 1.0));
                break;
            default: break;
            }
        }

        void Player::start()
        {
            timeAction(TimeAction::Start);
        }

        void Player::end()
        {
            timeAction(TimeAction::End);
        }

        void Player::framePrev()
        {
            timeAction(TimeAction::FramePrev);
        }

        void Player::frameNext()
        {
            timeAction(TimeAction::FrameNext);
        }

        const OTIO_NS::TimeRange& Player::getInOutRange() const
        {
            return _p->inOutRange->get();
        }

        std::shared_ptr<dtk::IObservableValue<OTIO_NS::TimeRange> > Player::observeInOutRange() const
        {
            return _p->inOutRange;
        }

        void Player::setInOutRange(const OTIO_NS::TimeRange& value)
        {
            DTK_P();
            if (p.inOutRange->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.inOutRange = value;
                p.mutex.clearRequests = true;
            }
        }

        void Player::setInPoint()
        {
            DTK_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time(
                p.currentTime->get(),
                p.inOutRange->get().end_time_exclusive()));
        }

        void Player::resetInPoint()
        {
            DTK_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time(
                p.timeRange.start_time(),
                p.inOutRange->get().end_time_exclusive()));
        }

        void Player::setOutPoint()
        {
            DTK_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                p.inOutRange->get().start_time(),
                p.currentTime->get()));
        }

        void Player::resetOutPoint()
        {
            DTK_P();
            setInOutRange(OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                p.inOutRange->get().start_time(),
                p.timeRange.end_time_inclusive()));
        }

        const std::vector<std::shared_ptr<Timeline> >& Player::getCompare() const
        {
            return _p->compare->get();
        }

        std::shared_ptr<dtk::IObservableList<std::shared_ptr<Timeline> > > Player::observeCompare() const
        {
            return _p->compare;
        }

        void Player::setCompare(const std::vector<std::shared_ptr<Timeline> >& value)
        {
            DTK_P();
            if (p.compare->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.compare = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        CompareTime Player::getCompareTime() const
        {
            return _p->compareTime->get();
        }

        std::shared_ptr<dtk::IObservableValue<CompareTime> > Player::observeCompareTime() const
        {
            return _p->compareTime;
        }

        void Player::setCompareTime(CompareTime value)
        {
            DTK_P();
            if (p.compareTime->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.compareTime = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const io::Options& Player::getIOOptions() const
        {
            return _p->ioOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<io::Options> > Player::observeIOOptions() const
        {
            return _p->ioOptions;
        }

        void Player::setIOOptions(const io::Options& value)
        {
            DTK_P();
            if (p.ioOptions->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.ioOptions = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        int Player::getVideoLayer() const
        {
            return _p->videoLayer->get();
        }

        std::shared_ptr<dtk::IObservableValue<int> > Player::observeVideoLayer() const
        {
            return _p->videoLayer;
        }

        void Player::setVideoLayer(int value)
        {
            DTK_P();
            if (p.videoLayer->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.videoLayer = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const std::vector<int>& Player::getCompareVideoLayers() const
        {
            return _p->compareVideoLayers->get();
        }

        std::shared_ptr<dtk::IObservableList<int> > Player::observeCompareVideoLayers() const
        {
            return _p->compareVideoLayers;
        }

        void Player::setCompareVideoLayers(const std::vector<int>& value)
        {
            DTK_P();
            if (p.compareVideoLayers->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.compareVideoLayers = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const std::vector<VideoData>& Player::getCurrentVideo() const
        {
            return _p->currentVideoData->get();
        }

        std::shared_ptr<dtk::IObservableList<VideoData> > Player::observeCurrentVideo() const
        {
            return _p->currentVideoData;
        }

        const PlayerCacheOptions& Player::getCacheOptions() const
        {
            return _p->cacheOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<PlayerCacheOptions> > Player::observeCacheOptions() const
        {
            return _p->cacheOptions;
        }

        void Player::setCacheOptions(const PlayerCacheOptions& value)
        {
            DTK_P();
            if (p.cacheOptions->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.cacheOptions = value;
            }
        }

        std::shared_ptr<dtk::IObservableValue<PlayerCacheInfo> > Player::observeCacheInfo() const
        {
            return _p->cacheInfo;
        }

        void Player::clearCache()
        {
            DTK_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            p.mutex.clearRequests = true;
            p.mutex.clearCache = true;
        }

        void Player::tick()
        {
            DTK_P();

            // Tick the timeline.
            p.timeline->tick();

            // Calculate the current time.
            const double timelineSpeed = p.timeRange.duration().rate();
            const auto playback = p.playback->get();
            if (playback != Playback::Stop && timelineSpeed > 0.0)
            {
                OTIO_NS::RationalTime start = time::invalidTime;
                double t = 0.0;
                if (p.hasAudio())
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    start = p.audioMutex.start;
                    t = OTIO_NS::RationalTime(p.audioMutex.frame, p.ioInfo.audio.sampleRate).rescaled_to(1.0).value();
                }
                else
                {
                    start = p.noAudio.start;
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<double> diff = now - p.noAudio.playbackTimer;
                    t = diff.count() * p.speed->get() / timelineSpeed;
                }
                if (Playback::Reverse == playback)
                {
                    t = -t;
                }
                bool looped = false;
                const OTIO_NS::RationalTime currentTime = p.loopPlayback(
                    start +
                    OTIO_NS::RationalTime(t, 1.0).rescaled_to(timelineSpeed).floor(),
                    looped);
                //const double currentTimeDiff = abs(currentTime.value() - p.currentTime->get().value());
                if (p.currentTime->setIfChanged(currentTime))
                {
                    //std::cout << "current time: " << p.currentTime->get() << " / " << currentTimeDiff << std::endl;
                    if (looped)
                    {
                        p.seek->setAlways(currentTime);
                    }
                }
            }

            // Sync with the thread.
            std::vector<VideoData> currentVideoData;
            std::vector<AudioData> currentAudioData;
            PlayerCacheInfo cacheInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.state.currentTime = p.currentTime->get();
                currentVideoData = p.mutex.currentVideoData;
                currentAudioData = p.mutex.currentAudioData;
                cacheInfo = p.mutex.cacheInfo;
            }
            p.currentVideoData->setIfChanged(currentVideoData);
            p.currentAudioData->setIfChanged(currentAudioData);
            p.cacheInfo->setIfChanged(cacheInfo);
        }

        void Player::_thread()
        {
            DTK_P();
            p.thread.cacheTimer = std::chrono::steady_clock::now();
            p.thread.logTimer = std::chrono::steady_clock::now();
            while (p.running)
            {
                const auto t0 = std::chrono::steady_clock::now();

                // Get mutex protected values.
                Private::PlaybackState state;
                bool clearRequests = false;
                bool clearCache = false;
                CacheDirection cacheDirection = CacheDirection::First;
                PlayerCacheOptions cacheOptions;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    state = p.mutex.state;
                    clearRequests = p.mutex.clearRequests;
                    p.mutex.clearRequests = false;
                    clearCache = p.mutex.clearCache;
                    p.mutex.clearCache = false;
                    cacheDirection = p.mutex.cacheDirection;
                }
                if (state != p.thread.state ||
                    clearRequests ||
                    clearCache ||
                    cacheDirection != p.thread.cacheDirection)
                {
                    p.thread.state = state;
                    p.thread.cacheDirection = cacheDirection;
                    p.thread.videoDataCacheFrame = 0;
                    p.thread.videoDataCacheFill = 0;
                    p.thread.audioDataCacheSeconds = 0;
                    p.thread.audioDataCacheFill = 0;
                }

                // Clear requests.
                if (clearRequests)
                {
                    p.clearRequests();
                }

                // Clear the cache.
                if (clearCache)
                {
                    p.clearCache();
                }

                // Update the cache.
                p.cacheUpdate();

                // Update the current video data.
                if (!p.ioInfo.video.empty())
                {
                    std::vector<VideoData> videoDataList;
                    if (p.thread.videoDataCache.get(p.thread.state.currentTime, videoDataList))
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.currentVideoData = videoDataList;
                    }
                    else if (p.thread.state.playback != Playback::Stop)
                    {
                        if (!p.timeRange.contains(p.thread.state.currentTime))
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            p.mutex.currentVideoData.clear();
                        }
                        const auto now = std::chrono::steady_clock::now();
                        if (now > p.audioMutex.state.muteTimeout)
                        {
                            {
                                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                                p.audioMutex.state.muteTimeout = now + p.playerOptions.muteTimeout;
                                p.audioReset(p.currentTime->get());
                            }
                            if (!p.hasAudio())
                            {
                                p.playbackReset(p.currentTime->get());
                            }
                        }
                    }
                    else
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        if (!p.timeRange.contains(p.thread.state.currentTime))
                        {
                            p.mutex.currentVideoData.clear();
                        }
                    }
                }

                // Update the current audio data.
                if (p.ioInfo.audio.isValid())
                {
                    std::vector<AudioData> audioDataList;
                    {
                        const int64_t seconds = p.thread.state.currentTime.rescaled_to(1.0).value() -
                            p.thread.state.audioOffset -
                            p.timeRange.start_time().rescaled_to(1.0).value();
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        for (int64_t s : { seconds - 1, seconds, seconds + 1 })
                        {
                            AudioData audioData;
                            if (p.audioMutex.audioDataCache.get(s, audioData))
                            {
                                audioDataList.push_back(audioData);
                            }
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.currentAudioData = audioDataList;
                    }
                }

                // Logging.
                auto t1 = std::chrono::steady_clock::now();
                const std::chrono::duration<double> diff = t1 - p.thread.logTimer;
                if (diff.count() > 10.0)
                {
                    p.thread.logTimer = t1;
                    if (auto context = getContext())
                    {
                        p.log(context);
                    }
                    t1 = std::chrono::steady_clock::now();
                }

                // Sleep for a bit.
                dtk::sleep(p.playerOptions.sleepTimeout, t0, t1);
            }
            p.clearRequests();
        }
    }
}
