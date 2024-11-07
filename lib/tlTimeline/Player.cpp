// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(Playback, "Stop", "Forward", "Reverse");
        TLRENDER_ENUM_SERIALIZE_IMPL(Playback);

        TLRENDER_ENUM_IMPL(Loop, "Loop", "Once", "Ping-Pong");
        TLRENDER_ENUM_SERIALIZE_IMPL(Loop);

        TLRENDER_ENUM_IMPL(TimeAction,
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
        TLRENDER_ENUM_SERIALIZE_IMPL(TimeAction);

        void Player::_init(
            const std::shared_ptr<Timeline>& timeline,
            const std::shared_ptr<system::Context>& context,
            const PlayerOptions& playerOptions)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(string::Format("    Cache read ahead: {0}").
                    arg(playerOptions.cache.readAhead));
                lines.push_back(string::Format("    Cache read behind: {0}").
                    arg(playerOptions.cache.readBehind));
                lines.push_back(string::Format("    Audio buffer frame count: {0}").
                    arg(playerOptions.audioBufferFrameCount));
                lines.push_back(string::Format("    Mute timeout: {0}ms").
                    arg(playerOptions.muteTimeout.count()));
                lines.push_back(string::Format("    Sleep timeout: {0}ms").
                    arg(playerOptions.sleepTimeout.count()));
                logSystem->print(
                    string::Format("tl::timeline::Player {0}").arg(this),
                    string::join(lines, "\n"));
            }

            p.playerOptions = playerOptions;
            p.timeline = timeline;
            p.timeRange = timeline->getTimeRange();
            p.ioInfo = timeline->getIOInfo();

            // Create observers.
            p.speed = observer::Value<double>::create(p.timeRange.duration().rate());
            p.playback = observer::Value<Playback>::create(Playback::Stop);
            p.loop = observer::Value<Loop>::create(Loop::Loop);
            p.currentTime = observer::Value<otime::RationalTime>::create(
                playerOptions.currentTime != time::invalidTime ?
                playerOptions.currentTime :
                p.timeRange.start_time());
            p.seek = observer::Value<otime::RationalTime>::create(p.currentTime->get());
            p.inOutRange = observer::Value<otime::TimeRange>::create(p.timeRange);
            p.compare = observer::List<std::shared_ptr<Timeline> >::create();
            p.compareTime = observer::Value<CompareTimeMode>::create(CompareTimeMode::Relative);
            p.ioOptions = observer::Value<io::Options>::create();
            p.videoLayer = observer::Value<int>::create(0);
            p.compareVideoLayers = observer::List<int>::create();
            p.currentVideoData = observer::List<VideoData>::create();
            p.audioDevice = observer::Value<audio::DeviceID>::create(playerOptions.audioDevice);
            p.volume = observer::Value<float>::create(1.F);
            p.mute = observer::Value<bool>::create(false);
            p.audioOffset = observer::Value<double>::create(0.0);
            p.currentAudioData = observer::List<AudioData>::create();
            p.cacheOptions = observer::Value<PlayerCacheOptions>::create(playerOptions.cache);
            p.cacheInfo = observer::Value<PlayerCacheInfo>::create();
            auto weak = std::weak_ptr<Player>(shared_from_this());
            p.timelineObserver = observer::ValueObserver<bool>::create(
                p.timeline->observeTimelineChanges(),
                [weak](bool)
                {
                    if (auto player = weak.lock())
                    {
                        player->clearCache();
                    }
                });
            auto audioSystem = context->getSystem<audio::System>();
            p.audioDevicesObserver = observer::ListObserver<audio::DeviceInfo>::create(
                audioSystem->observeDevices(),
                [weak](const std::vector<audio::DeviceInfo>&)
                {
                    if (auto player = weak.lock())
                    {
                        if (auto context = player->getContext().lock())
                        {
                            player->_p->audioInit(context);
                        }
                    }
                });
            p.defaultAudioDeviceObserver = observer::ValueObserver<audio::DeviceInfo>::create(
                audioSystem->observeDefaultDevice(),
                [weak](const audio::DeviceInfo&)
                {
                    if (auto player = weak.lock())
                    {
                        if (audio::DeviceID() == player->_p->audioDevice->get())
                        {
                            if (auto context = player->getContext().lock())
                            {
                                player->_p->audioInit(context);
                            }
                        }
                    }
                });

            // Initialize the audio.
            p.audioInit(context);

            // Create a new thread.
            p.mutex.currentTime = p.currentTime->get();
            p.mutex.inOutRange = p.inOutRange->get();
            p.mutex.audioOffset = p.audioOffset->get();
            p.mutex.cacheOptions = p.cacheOptions->get();
            p.mutex.cacheInfo = p.cacheInfo->get();
            p.audioMutex.speed = p.speed->get();
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
            TLRENDER_P();
            p.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
#if defined(TLRENDER_SDL2)
            if (p.sdlID > 0)
            {
                SDL_CloseAudioDevice(p.sdlID);
            }
#elif defined(TLRENDER_SDL3)
            if (p.sdlStream)
            {
                SDL_DestroyAudioStream(p.sdlStream);
            }
#endif // TLRENDER_SDL2
        }

        std::shared_ptr<Player> Player::create(
            const std::shared_ptr<Timeline>& timeline,
            const std::shared_ptr<system::Context>& context,
            const PlayerOptions& playerOptions)
        {
            auto out = std::shared_ptr<Player>(new Player);
            out->_init(timeline, context, playerOptions);
            return out;
        }

        const std::weak_ptr<system::Context>& Player::getContext() const
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

        const otime::TimeRange& Player::getTimeRange() const
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

        std::shared_ptr<observer::IValue<double> > Player::observeSpeed() const
        {
            return _p->speed;
        }

        void Player::setSpeed(double value)
        {
            TLRENDER_P();
            if (p.speed->setIfChanged(value))
            {
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    p.audioMutex.speed = value;
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

        std::shared_ptr<observer::IValue<Playback> > Player::observePlayback() const
        {
            return _p->playback;
        }

        void Player::setPlayback(Playback value)
        {
            TLRENDER_P();

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
                        p.mutex.playback = value;
                        p.mutex.currentTime = p.currentTime->get();
                        p.mutex.cacheDirection = Playback::Forward == value ?
                            CacheDirection::Forward :
                            CacheDirection::Reverse;
                        p.mutex.clearRequests = true;
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.playback = value;
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
                        p.mutex.playback = value;
                        p.mutex.clearRequests = true;
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.playback = value;
                    }
                }
            }
        }

        Loop Player::getLoop() const
        {
            return _p->loop->get();
        }

        std::shared_ptr<observer::IValue<Loop> > Player::observeLoop() const
        {
            return _p->loop;
        }

        void Player::setLoop(Loop value)
        {
            _p->loop->setIfChanged(value);
        }

        otime::RationalTime Player::getCurrentTime() const
        {
            return _p->currentTime->get();
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> > Player::observeCurrentTime() const
        {
            return _p->currentTime;
        }

        std::shared_ptr<observer::IValue<otime::RationalTime> > Player::observeSeek() const
        {
            return _p->seek;
        }

        void Player::seek(const otime::RationalTime& time)
        {
            TLRENDER_P();

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
                    p.mutex.currentTime = tmp;
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
            TLRENDER_P();
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
                seek(currentTime - otime::RationalTime(1, p.timeRange.duration().rate()));
                break;
            case TimeAction::FramePrevX10:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime - otime::RationalTime(10, p.timeRange.duration().rate()));
                break;
            case TimeAction::FramePrevX100:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime - otime::RationalTime(100, p.timeRange.duration().rate()));
                break;
            case TimeAction::FrameNext:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime + otime::RationalTime(1, p.timeRange.duration().rate()));
                break;
            case TimeAction::FrameNextX10:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime + otime::RationalTime(10, p.timeRange.duration().rate()));
                break;
            case TimeAction::FrameNextX100:
                setPlayback(timeline::Playback::Stop);
                seek(currentTime + otime::RationalTime(100, p.timeRange.duration().rate()));
                break;
            case TimeAction::JumpBack1s:
                seek(currentTime - otime::RationalTime(1.0, 1.0));
                break;
            case TimeAction::JumpBack10s:
                seek(currentTime - otime::RationalTime(10.0, 1.0));
                break;
            case TimeAction::JumpForward1s:
                seek(currentTime + otime::RationalTime(1.0, 1.0));
                break;
            case TimeAction::JumpForward10s:
                seek(currentTime + otime::RationalTime(10.0, 1.0));
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

        otime::TimeRange Player::getInOutRange() const
        {
            return _p->inOutRange->get();
        }

        std::shared_ptr<observer::IValue<otime::TimeRange> > Player::observeInOutRange() const
        {
            return _p->inOutRange;
        }

        void Player::setInOutRange(const otime::TimeRange& value)
        {
            TLRENDER_P();
            if (p.inOutRange->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.inOutRange = value;
                p.mutex.clearRequests = true;
            }
        }

        void Player::setInPoint()
        {
            TLRENDER_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time(
                p.currentTime->get(),
                p.inOutRange->get().end_time_exclusive()));
        }

        void Player::resetInPoint()
        {
            TLRENDER_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time(
                p.timeRange.start_time(),
                p.inOutRange->get().end_time_exclusive()));
        }

        void Player::setOutPoint()
        {
            TLRENDER_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                p.inOutRange->get().start_time(),
                p.currentTime->get()));
        }

        void Player::resetOutPoint()
        {
            TLRENDER_P();
            setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                p.inOutRange->get().start_time(),
                p.timeRange.end_time_inclusive()));
        }

        const std::vector<std::shared_ptr<Timeline> >& Player::getCompare() const
        {
            return _p->compare->get();
        }

        std::shared_ptr<observer::IList<std::shared_ptr<Timeline> > > Player::observeCompare() const
        {
            return _p->compare;
        }

        void Player::setCompare(const std::vector<std::shared_ptr<Timeline> >& value)
        {
            TLRENDER_P();
            if (p.compare->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compare = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        CompareTimeMode Player::getCompareTime() const
        {
            return _p->compareTime->get();
        }

        std::shared_ptr<observer::IValue<CompareTimeMode> > Player::observeCompareTime() const
        {
            return _p->compareTime;
        }

        void Player::setCompareTime(CompareTimeMode value)
        {
            TLRENDER_P();
            if (p.compareTime->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compareTime = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const io::Options& Player::getIOOptions() const
        {
            return _p->ioOptions->get();
        }

        std::shared_ptr<observer::IValue<io::Options> > Player::observeIOOptions() const
        {
            return _p->ioOptions;
        }

        void Player::setIOOptions(const io::Options& value)
        {
            TLRENDER_P();
            if (p.ioOptions->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.ioOptions = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        int Player::getVideoLayer() const
        {
            return _p->videoLayer->get();
        }

        std::shared_ptr<observer::IValue<int> > Player::observeVideoLayer() const
        {
            return _p->videoLayer;
        }

        void Player::setVideoLayer(int value)
        {
            TLRENDER_P();
            if (p.videoLayer->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.videoLayer = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const std::vector<int>& Player::getCompareVideoLayers() const
        {
            return _p->compareVideoLayers->get();
        }

        std::shared_ptr<observer::IList<int> > Player::observeCompareVideoLayers() const
        {
            return _p->compareVideoLayers;
        }

        void Player::setCompareVideoLayers(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (p.compareVideoLayers->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.compareVideoLayers = value;
                p.mutex.clearRequests = true;
                p.mutex.clearCache = true;
            }
        }

        const std::vector<VideoData>& Player::getCurrentVideo() const
        {
            return _p->currentVideoData->get();
        }

        std::shared_ptr<observer::IList<VideoData> > Player::observeCurrentVideo() const
        {
            return _p->currentVideoData;
        }

        const PlayerCacheOptions& Player::getCacheOptions() const
        {
            return _p->cacheOptions->get();
        }

        std::shared_ptr<observer::IValue<PlayerCacheOptions> > Player::observeCacheOptions() const
        {
            return _p->cacheOptions;
        }

        void Player::setCacheOptions(const PlayerCacheOptions& value)
        {
            TLRENDER_P();
            if (p.cacheOptions->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.cacheOptions = value;
            }
        }

        std::shared_ptr<observer::IValue<PlayerCacheInfo> > Player::observeCacheInfo() const
        {
            return _p->cacheInfo;
        }

        void Player::clearCache()
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            p.mutex.clearRequests = true;
            p.mutex.clearCache = true;
        }

        void Player::tick()
        {
            TLRENDER_P();

            // Tick the timeline.
            p.timeline->tick();

            // Calculate the current time.
            const double timelineSpeed = p.timeRange.duration().rate();
            const auto playback = p.playback->get();
            if (playback != Playback::Stop && timelineSpeed > 0.0)
            {
                otime::RationalTime start = time::invalidTime;
                double t = 0.0;
                if (p.hasAudio())
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    start = p.audioMutex.start;
                    t = otime::RationalTime(p.audioMutex.frame, p.ioInfo.audio.sampleRate).rescaled_to(1.0).value();
                }
                else
                {
                    start = p.noAudio.start;
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<double> diff = now - p.noAudio.playbackTimer;
                    t = diff.count();
                }
                if (Playback::Reverse == playback)
                {
                    t = -t;
                }
                const double speedMult = p.speed->get() / timelineSpeed;
                bool looped = false;
                const otime::RationalTime currentTime = p.loopPlayback(
                    start +
                    otime::RationalTime(t * speedMult, 1.0).rescaled_to(timelineSpeed).floor(),
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
                p.mutex.currentTime = p.currentTime->get();
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
            TLRENDER_P();
            p.thread.cacheTimer = std::chrono::steady_clock::now();
            p.thread.logTimer = std::chrono::steady_clock::now();
            while (p.running)
            {
                const auto t0 = std::chrono::steady_clock::now();

                // Get mutex protected values.
                std::vector<std::shared_ptr<Timeline> > compare;
                bool clearRequests = false;
                bool clearCache = false;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.thread.playback = p.mutex.playback;
                    p.thread.currentTime = p.mutex.currentTime;
                    p.thread.inOutRange = p.mutex.inOutRange;
                    compare = p.mutex.compare;
                    p.thread.compareTime = p.mutex.compareTime;
                    p.thread.ioOptions = p.mutex.ioOptions;
                    p.thread.videoLayer = p.mutex.videoLayer;
                    p.thread.compareVideoLayers = p.mutex.compareVideoLayers;
                    p.thread.audioOffset = p.mutex.audioOffset;
                    clearRequests = p.mutex.clearRequests;
                    p.mutex.clearRequests = false;
                    clearCache = p.mutex.clearCache;
                    p.mutex.clearCache = false;
                    p.thread.cacheDirection = p.mutex.cacheDirection;
                    p.thread.cacheOptions = p.mutex.cacheOptions;
                }

                // Clear requests.
                if (clearRequests)
                {
                    p.clearRequests();
                }
                p.thread.compare = compare;

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
                    const auto i = p.thread.videoDataCache.find(p.thread.currentTime);
                    if (i != p.thread.videoDataCache.end())
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.currentVideoData = i->second;
                    }
                    else if (p.thread.playback != Playback::Stop)
                    {
                        if (!p.timeRange.contains(p.thread.currentTime))
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            p.mutex.currentVideoData.clear();
                        }
                        const auto now = std::chrono::steady_clock::now();
                        if (now > p.audioMutex.muteTimeout)
                        {
                            {
                                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                                p.audioMutex.muteTimeout = now + p.playerOptions.muteTimeout;
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
                        if (!p.timeRange.contains(p.thread.currentTime))
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
                        const int64_t seconds = p.thread.currentTime.rescaled_to(1.0).value() -
                            p.thread.audioOffset -
                            p.timeRange.start_time().rescaled_to(1.0).value();
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        for (int64_t s : { seconds - 1, seconds, seconds + 1 })
                        {
                            auto i = p.audioMutex.audioDataCache.find(s);
                            if (i != p.audioMutex.audioDataCache.end())
                            {
                                audioDataList.push_back(i->second);
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
                    if (auto context = getContext().lock())
                    {
                        p.log(context);
                    }
                    t1 = std::chrono::steady_clock::now();
                }

                // Sleep for a bit.
                time::sleep(p.playerOptions.sleepTimeout, t0, t1);
            }
            p.clearRequests();
        }
    }
}
