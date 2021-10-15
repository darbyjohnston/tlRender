// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/TimelinePlayer.h>

#include <tlrCore/Error.h>
#include <tlrCore/File.h>
#include <tlrCore/LogSystem.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/stackAlgorithm.h>
#include <opentimelineio/timeline.h>

#if defined(TLR_ENABLE_PYTHON)
#include <Python.h>
#endif

#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace tlr
{
    namespace timeline
    {
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

        otime::RationalTime loopTime(const otime::RationalTime& time, const otime::TimeRange& range)
        {
            auto out = time;
            if (out < range.start_time())
            {
                out = range.end_time_inclusive();
            }
            else if (out > range.end_time_inclusive())
            {
                out = range.start_time();
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
                CacheDirection,
                std::size_t cacheReadAhead,
                std::size_t cacheReadBehind);

            std::shared_ptr<Timeline> timeline;

            std::shared_ptr<observer::Value<float> > speed;
            std::shared_ptr<observer::Value<Playback> > playback;
            std::shared_ptr<observer::Value<Loop> > loop;
            std::shared_ptr<observer::Value<otime::RationalTime> > currentTime;
            std::shared_ptr<observer::Value<otime::TimeRange> > inOutRange;
            std::shared_ptr<observer::Value<uint16_t> > videoLayer;
            std::shared_ptr<observer::Value<VideoData> > video;
            std::shared_ptr<observer::Value<float> > cachePercentage;
            std::shared_ptr<observer::List<otime::TimeRange> > cachedFrames;
            std::chrono::steady_clock::time_point prevTime;

            struct ThreadData
            {
                otime::RationalTime currentTime = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                uint16_t videoLayer = 0;
                VideoData videoData;
                AudioData audioData;
                std::map<otime::RationalTime, std::future<VideoData> > videoDataRequests;
                std::map<otime::RationalTime, std::future<AudioData> > audioDataRequests;
                bool clearRequests = false;
                std::map<otime::RationalTime, VideoData> videoDataCache;
                std::map<otime::RationalTime, AudioData> audioDataCache;
                std::vector<otime::TimeRange> cachedFrames;
                bool clearCache = false;
                CacheDirection cacheDirection = CacheDirection::Forward;
                std::size_t cacheReadAhead = 100;
                std::size_t cacheReadBehind = 10;
                std::mutex mutex;
                std::atomic<bool> running;
            };
            ThreadData threadData;
            std::thread thread;

            std::chrono::steady_clock::time_point logTimer;
        };

        void TimelinePlayer::_init(
            const file::Path& path,
            const std::shared_ptr<core::Context>& context,
            const Options& options)
        {
            TLR_PRIVATE_P();

            // Create the timeline.
            p.timeline = timeline::Timeline::create(path, context, options);

            // Create observers.
            p.speed = observer::Value<float>::create(p.timeline->getDuration().rate());
            p.playback = observer::Value<Playback>::create(Playback::Stop);
            p.loop = observer::Value<Loop>::create(Loop::Loop);
            p.currentTime = observer::Value<otime::RationalTime>::create(p.timeline->getGlobalStartTime());
            p.inOutRange = observer::Value<otime::TimeRange>::create(
                otime::TimeRange(p.timeline->getGlobalStartTime(), p.timeline->getDuration()));
            p.videoLayer = observer::Value<uint16_t>::create();
            p.video = observer::Value<VideoData>::create();
            p.cachePercentage = observer::Value<float>::create();
            p.cachedFrames = observer::List<otime::TimeRange>::create();

            // Create a new thread.
            p.threadData.currentTime = p.currentTime->get();
            p.threadData.inOutRange = p.inOutRange->get();
            p.threadData.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLR_PRIVATE_P();
                    p.logTimer = std::chrono::steady_clock::now();
                    while (p.threadData.running)
                    {
                        otime::RationalTime currentTime = time::invalidTime;
                        otime::TimeRange inOutRange = time::invalidTimeRange;
                        uint16_t videoLayer = 0;
                        bool clearRequests = false;
                        bool clearCache = false;
                        CacheDirection cacheDirection = CacheDirection::Forward;
                        std::size_t cacheReadAhead = 0;
                        std::size_t cacheReadBehind = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.threadData.mutex);
                            currentTime = p.threadData.currentTime;
                            inOutRange = p.threadData.inOutRange;
                            videoLayer = p.threadData.videoLayer;
                            clearRequests = p.threadData.clearRequests;
                            p.threadData.clearRequests = false;
                            clearCache = p.threadData.clearCache;
                            p.threadData.clearCache = false;
                            cacheDirection = p.threadData.cacheDirection;
                            cacheReadAhead = p.threadData.cacheReadAhead;
                            cacheReadBehind = p.threadData.cacheReadBehind;
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
                            p.threadData.audioDataCache.clear();
                            p.threadData.cachedFrames.clear();
                        }

                        // Update the cache.
                        p.cacheUpdate(
                            currentTime,
                            inOutRange,
                            videoLayer,
                            cacheDirection,
                            cacheReadAhead,
                            cacheReadBehind);

                        // Update the video data.
                        const auto i = p.threadData.videoDataCache.find(currentTime);
                        if (i != p.threadData.videoDataCache.end())
                        {
                            std::unique_lock<std::mutex> lock(p.threadData.mutex);
                            p.threadData.videoData = i->second;
                        }

                        // Logging.
                        const auto now = std::chrono::steady_clock::now();
                        const std::chrono::duration<float> diff = now - p.logTimer;
                        if (diff.count() > 10.F)
                        {
                            p.logTimer = now;
                            if (auto context = getContext().lock())
                            {
                                const std::string id = string::Format("tlr::timeline::TimelinePlayer {0}").arg(this);
                                auto logSystem = context->getLogSystem();
                                logSystem->print(id, string::Format(
                                    "\n"
                                    "    path: {0}\n"
                                    "    time: {1}\n"
                                    "    in/out: {2}\n"
                                    "    video layer: {3}\n"
                                    "    video: {4}/{5} (requests/cache)\n"
                                    "    audio: {6}/{7} (requests/cache)\n").
                                    arg(getPath().get()).
                                    arg(currentTime).
                                    arg(inOutRange).
                                    arg(videoLayer).
                                    arg(p.threadData.videoDataRequests.size()).
                                    arg(p.threadData.videoDataCache.size()).
                                    arg(p.threadData.audioDataRequests.size()).
                                    arg(p.threadData.audioDataCache.size()));
                            }
                        }

                        time::sleep(std::chrono::microseconds(1000));
                    }
                });
        }

        TimelinePlayer::TimelinePlayer() :
            _p(new Private)
        {}

        TimelinePlayer::~TimelinePlayer()
        {
            TLR_PRIVATE_P();
            p.threadData.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<TimelinePlayer> TimelinePlayer::create(
            const file::Path& path,
            const std::shared_ptr<core::Context>& context,
            const Options& options)
        {
            auto out = std::shared_ptr<TimelinePlayer>(new TimelinePlayer);
            out->_init(path, context, options);
            return out;
        }

        const std::weak_ptr<core::Context>& TimelinePlayer::getContext() const
        {
            return _p->timeline->getContext();
        }
        
        const otio::SerializableObject::Retainer<otio::Timeline>& TimelinePlayer::getTimeline() const
        {
            return _p->timeline->getTimeline();
        }
        
        const file::Path& TimelinePlayer::getPath() const
        {
            return _p->timeline->getPath();
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
            return _p->timeline->getAVInfo();
        }

        float TimelinePlayer::getDefaultSpeed() const
        {
            return _p->timeline->getDuration().rate();
        }

        std::shared_ptr<observer::IValue<float> > TimelinePlayer::observeSpeed() const
        {
            return _p->speed;
        }

        void TimelinePlayer::setSpeed(float value)
        {
            _p->speed->setIfChanged(value);
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
                    p.prevTime = std::chrono::steady_clock::now();

                    std::unique_lock<std::mutex> lock(p.threadData.mutex);
                    p.threadData.cacheDirection = Playback::Forward == value ? CacheDirection::Forward : CacheDirection::Reverse;
                }
                else
                {
                    std::unique_lock<std::mutex> lock(p.threadData.mutex);
                    p.threadData.clearRequests = true;
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
            const auto tmp = loopTime(time, range);

            if (p.currentTime->setIfChanged(tmp))
            {
                //std::cout << "! " << tmp << std::endl;

                // Update playback.
                if (p.playback->get() != Playback::Stop)
                {
                    p.prevTime = std::chrono::steady_clock::now();
                }

                {
                    std::unique_lock<std::mutex> lock(p.threadData.mutex);
                    p.threadData.currentTime = tmp;
                    p.threadData.clearRequests = true;
                }
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
                seek(otime::RationalTime(currentTime.value() - 1, duration.rate()));
                break;
            case TimeAction::FramePrevX10:
                seek(otime::RationalTime(currentTime.value() - 10, duration.rate()));
                break;
            case TimeAction::FramePrevX100:
                seek(otime::RationalTime(currentTime.value() - 100, duration.rate()));
                break;
            case TimeAction::FrameNext:
                seek(otime::RationalTime(currentTime.value() + 1, duration.rate()));
                break;
            case TimeAction::FrameNextX10:
                seek(otime::RationalTime(currentTime.value() + 10, duration.rate()));
                break;
            case TimeAction::FrameNextX100:
                seek(otime::RationalTime(currentTime.value() + 100, duration.rate()));
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

        std::shared_ptr<observer::IValue<otime::TimeRange> > TimelinePlayer::observeInOutRange() const
        {
            return _p->inOutRange;
        }

        void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
        {
            TLR_PRIVATE_P();
            if (p.inOutRange->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.threadData.mutex);
                p.threadData.inOutRange = value;
                p.threadData.clearRequests = true;
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
                std::unique_lock<std::mutex> lock(p.threadData.mutex);
                p.threadData.videoLayer = layer;
                p.threadData.clearCache = true;
            }
        }

        std::shared_ptr<observer::IValue<VideoData> > TimelinePlayer::observeVideo() const
        {
            return _p->video;
        }

        size_t TimelinePlayer::getCacheReadAhead()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            return p.threadData.cacheReadAhead;
        }

        size_t TimelinePlayer::getCacheReadBehind()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            return p.threadData.cacheReadBehind;
        }

        void TimelinePlayer::setCacheReadAhead(size_t value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            p.threadData.cacheReadAhead = value;
        }

        void TimelinePlayer::setCacheReadBehind(size_t value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            p.threadData.cacheReadBehind = value;
        }

        std::shared_ptr<observer::IValue<float> > TimelinePlayer::observeCachePercentage() const
        {
            return _p->cachePercentage;
        }

        std::shared_ptr<observer::IList<otime::TimeRange> > TimelinePlayer::observeCachedFrames() const
        {
            return _p->cachedFrames;
        }

        void TimelinePlayer::tick()
        {
            TLR_PRIVATE_P();

            // Calculate the current time.
            const auto playback = p.playback->get();
            if (playback != Playback::Stop)
            {
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - p.prevTime;
                const float speed = p.speed->get();
                const auto& duration = p.timeline->getDuration();
                otime::RationalTime delta;
                if (Playback::Forward == playback)
                {
                    delta = otime::RationalTime(floor(diff.count() * speed), duration.rate());
                }
                else
                {
                    delta = otime::RationalTime(ceil(diff.count() * speed * -1.0), duration.rate());
                }
                const auto currentTime = p.loopPlayback(p.currentTime->get() + delta);
                if (p.currentTime->setIfChanged(currentTime))
                {
                    p.prevTime = now;
                    //std::cout << "! " << p.currentTime->get() << std::endl;
                }
            }

            // Sync with the thread.
            VideoData videoData;
            AudioData audioData;
            int cacheReadAhead = 0;
            int cacheReadBehind = 0;
            std::vector<otime::TimeRange> cachedFrames;
            {
                std::unique_lock<std::mutex> lock(p.threadData.mutex);
                p.threadData.currentTime = p.currentTime->get();
                videoData = p.threadData.videoData;
                audioData = p.threadData.audioData;
                cacheReadAhead = p.threadData.cacheReadAhead;
                cacheReadBehind = p.threadData.cacheReadBehind;
                cachedFrames = p.threadData.cachedFrames;
            }
            p.video->setIfChanged(videoData);
            size_t cachedFramesCount = 0;
            for (const auto& i : cachedFrames)
            {
                cachedFramesCount += i.duration().value();
            }
            p.cachePercentage->setIfChanged(
                cachedFramesCount / static_cast<float>(cacheReadAhead + cacheReadBehind) * 100.F);
            p.cachedFrames->setIfChanged(cachedFrames);
        }

        otime::RationalTime TimelinePlayer::Private::loopPlayback(const otime::RationalTime& time)
        {
            otime::RationalTime out = time;

            const auto& range = inOutRange->get();
            switch (loop->get())
            {
            case Loop::Loop:
            {
                const auto tmp = loopTime(out, range);
                if (tmp != out)
                {
                    out = tmp;
                    prevTime = std::chrono::steady_clock::now();
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
                    prevTime = std::chrono::steady_clock::now();
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playbackValue)
                {
                    out = range.end_time_inclusive();
                    playback->setIfChanged(Playback::Reverse);
                    prevTime = std::chrono::steady_clock::now();
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
            CacheDirection cacheDirection,
            std::size_t cacheReadAhead,
            std::size_t cacheReadBehind)
        {
            // Get which frames should be cached.
            std::vector<otime::RationalTime> frames;
            const auto& duration = timeline->getDuration();
            auto time = currentTime;
            const auto& range = inOutRange;
            const size_t backwardsCount = CacheDirection::Forward == cacheDirection ?
                cacheReadBehind :
                (cacheReadAhead - 1);
            for (std::size_t i = 0; i < backwardsCount; ++i)
            {
                time = loopTime(time - otime::RationalTime(1, duration.rate()), range);
            }
            for (std::size_t i = 0; i < cacheReadBehind + cacheReadAhead; ++i)
            {
                if (!frames.empty() && time == frames[0])
                {
                    break;
                }
                frames.push_back(time);
                time = loopTime(time + otime::RationalTime(1, duration.rate()), range);
            }
            const auto ranges = toRanges(frames);
            timeline->setActiveRanges(ranges);

            // Remove old frames from the cache.
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
            auto audioDataCacheIt = threadData.audioDataCache.begin();
            while (audioDataCacheIt != threadData.audioDataCache.end())
            {
                bool old = true;
                for (const auto& i : ranges)
                {
                    if (i.contains(audioDataCacheIt->second.time))
                    {
                        old = false;
                        break;
                    }
                }
                if (old)
                {
                    audioDataCacheIt = threadData.audioDataCache.erase(audioDataCacheIt);
                    continue;
                }
                ++audioDataCacheIt;
            }

            // Find uncached frames.
            std::vector<otime::RationalTime> uncached;
            for (const auto& i : frames)
            {
                const auto j = threadData.videoDataCache.find(i);
                if (j == threadData.videoDataCache.end())
                {
                    const auto k = threadData.videoDataRequests.find(i);
                    if (k == threadData.videoDataRequests.end())
                    {
                        uncached.push_back(i);
                    }
                }
            }

            // Get uncached frames.
            for (const auto& i : uncached)
            {
                threadData.videoDataRequests[i] = timeline->getVideo(i, videoLayer);
            }
            auto videoDataRequestsIt = threadData.videoDataRequests.begin();
            while (videoDataRequestsIt != threadData.videoDataRequests.end())
            {
                if (videoDataRequestsIt->second.valid() &&
                    videoDataRequestsIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto frame = videoDataRequestsIt->second.get();
                    frame.time = videoDataRequestsIt->first;
                    threadData.videoDataCache[frame.time] = frame;
                    videoDataRequestsIt = threadData.videoDataRequests.erase(videoDataRequestsIt);
                    continue;
                }
                ++videoDataRequestsIt;
            }

            // Update cached frames.
            std::vector<otime::RationalTime> cachedFrames;
            for (const auto& i : threadData.videoDataCache)
            {
                cachedFrames.push_back(i.second.time);
            }
            {
                std::unique_lock<std::mutex> lock(threadData.mutex);
                threadData.cachedFrames = toRanges(cachedFrames);
            }
        }
    }
}
