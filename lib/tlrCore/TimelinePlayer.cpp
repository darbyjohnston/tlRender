// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/TimelinePlayer.h>

#include <tlrCore/Error.h>
#include <tlrCore/File.h>
#include <tlrCore/String.h>
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
            enum class FrameCacheDirection
            {
                Forward,
                Reverse
            };
        }

        struct TimelinePlayer::Private
        {
            otime::RationalTime loopPlayback(const otime::RationalTime&);

            void frameCacheUpdate(
                const otime::RationalTime& currentTime,
                const otime::TimeRange& inOutRange,
                uint16_t videoLayer,
                FrameCacheDirection,
                std::size_t frameCacheReadAhead,
                std::size_t frameCacheReadBehind);

            std::shared_ptr<Timeline> timeline;

            std::shared_ptr<observer::Value<float> > speed;
            std::shared_ptr<observer::Value<Playback> > playback;
            std::shared_ptr<observer::Value<Loop> > loop;
            std::shared_ptr<observer::Value<otime::RationalTime> > currentTime;
            std::shared_ptr<observer::Value<otime::TimeRange> > inOutRange;
            std::shared_ptr<observer::Value<uint16_t> > videoLayer;
            std::shared_ptr<observer::Value<Frame> > frame;
            std::shared_ptr<observer::Value<float> > frameCachePercentage;
            std::shared_ptr<observer::List<otime::TimeRange> > cachedFrames;
            std::chrono::steady_clock::time_point prevTime;

            struct ThreadData
            {
                otime::RationalTime currentTime = time::invalidTime;
                otime::TimeRange inOutRange = time::invalidTimeRange;
                uint16_t videoLayer = 0;
                Frame frame;
                std::map<otime::RationalTime, std::future<Frame> > frameRequests;
                bool clearFrameRequests = false;
                std::map<otime::RationalTime, Frame> frameCache;
                std::vector<otime::TimeRange> cachedFrames;
                bool clearFrameCache = false;
                FrameCacheDirection frameCacheDirection = FrameCacheDirection::Forward;
                std::size_t frameCacheReadAhead = 100;
                std::size_t frameCacheReadBehind = 10;
                std::mutex mutex;
                std::atomic<bool> running;
            };
            ThreadData threadData;
            std::thread thread;
        };

        void TimelinePlayer::_init(
            const file::Path& path,
            const std::shared_ptr<core::Context>& context)
        {
            TLR_PRIVATE_P();

            // Create the timeline.
            p.timeline = timeline::Timeline::create(path, context);

            // Create observers.
            p.speed = observer::Value<float>::create(p.timeline->getDuration().rate());
            p.playback = observer::Value<Playback>::create(Playback::Stop);
            p.loop = observer::Value<Loop>::create(Loop::Loop);
            p.currentTime = observer::Value<otime::RationalTime>::create(p.timeline->getGlobalStartTime());
            p.inOutRange = observer::Value<otime::TimeRange>::create(
                otime::TimeRange(p.timeline->getGlobalStartTime(), p.timeline->getDuration()));
            p.videoLayer = observer::Value<uint16_t>::create();
            p.frame = observer::Value<Frame>::create();
            p.frameCachePercentage = observer::Value<float>::create();
            p.cachedFrames = observer::List<otime::TimeRange>::create();

            // Create a new thread.
            p.threadData.currentTime = p.currentTime->get();
            p.threadData.inOutRange = p.inOutRange->get();
            p.threadData.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLR_PRIVATE_P();

                    while (p.threadData.running)
                    {
                        otime::RationalTime currentTime = time::invalidTime;
                        otime::TimeRange inOutRange = time::invalidTimeRange;
                        uint16_t videoLayer = 0;
                        bool clearFrameRequests = false;
                        bool clearFrameCache = false;
                        FrameCacheDirection frameCacheDirection = FrameCacheDirection::Forward;
                        std::size_t frameCacheReadAhead = 0;
                        std::size_t frameCacheReadBehind = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.threadData.mutex);
                            currentTime = p.threadData.currentTime;
                            inOutRange = p.threadData.inOutRange;
                            videoLayer = p.threadData.videoLayer;
                            clearFrameRequests = p.threadData.clearFrameRequests;
                            p.threadData.clearFrameRequests = false;
                            clearFrameCache = p.threadData.clearFrameCache;
                            p.threadData.clearFrameCache = false;
                            frameCacheDirection = p.threadData.frameCacheDirection;
                            frameCacheReadAhead = p.threadData.frameCacheReadAhead;
                            frameCacheReadBehind = p.threadData.frameCacheReadBehind;
                        }

                        //! Clear frame requests.
                        if (clearFrameRequests)
                        {
                            p.timeline->cancelFrames();
                            p.threadData.frameRequests.clear();
                        }

                        //! Clear the frame cache.
                        if (clearFrameCache)
                        {
                            p.threadData.frameCache.clear();
                            p.threadData.cachedFrames.clear();
                        }

                        //! Update the frame cache.
                        p.frameCacheUpdate(
                            currentTime,
                            inOutRange,
                            videoLayer,
                            frameCacheDirection,
                            frameCacheReadAhead,
                            frameCacheReadBehind);

                        //! Update the frame.
                        const auto i = p.threadData.frameCache.find(currentTime);
                        if (i != p.threadData.frameCache.end())
                        {
                            std::unique_lock<std::mutex> lock(p.threadData.mutex);
                            p.threadData.frame = i->second;
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
            const std::shared_ptr<core::Context>& context)
        {
            auto out = std::shared_ptr<TimelinePlayer>(new TimelinePlayer);
            out->_init(path, context);
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

        const otime::RationalTime& TimelinePlayer::getGlobalStartTime() const
        {
            return _p->timeline->getGlobalStartTime();
        }

        const otime::RationalTime& TimelinePlayer::getDuration() const
        {
            return _p->timeline->getDuration();
        }

        const std::vector<imaging::Info>& TimelinePlayer::getVideoInfo() const
        {
            return _p->timeline->getVideoInfo();
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
                    p.threadData.frameCacheDirection = Playback::Forward == value ? FrameCacheDirection::Forward : FrameCacheDirection::Reverse;
                }
                else
                {
                    std::unique_lock<std::mutex> lock(p.threadData.mutex);
                    p.threadData.clearFrameRequests = true;
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
                    p.threadData.clearFrameRequests = true;
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
                p.threadData.clearFrameRequests = true;
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
                p.threadData.clearFrameCache = true;
            }
        }

        std::shared_ptr<observer::IValue<Frame> > TimelinePlayer::observeFrame() const
        {
            return _p->frame;
        }

        size_t TimelinePlayer::getFrameCacheReadAhead()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            return p.threadData.frameCacheReadAhead;
        }

        size_t TimelinePlayer::getFrameCacheReadBehind()
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            return p.threadData.frameCacheReadBehind;
        }

        void TimelinePlayer::setFrameCacheReadAhead(size_t value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            p.threadData.frameCacheReadAhead = value;
        }

        void TimelinePlayer::setFrameCacheReadBehind(size_t value)
        {
            TLR_PRIVATE_P();
            std::unique_lock<std::mutex> lock(p.threadData.mutex);
            p.threadData.frameCacheReadBehind = value;
        }

        std::shared_ptr<observer::IValue<float> > TimelinePlayer::observeFrameCachePercentage() const
        {
            return _p->frameCachePercentage;
        }

        std::shared_ptr<observer::IList<otime::TimeRange> > TimelinePlayer::observeCachedFrames() const
        {
            return _p->cachedFrames;
        }

        size_t TimelinePlayer::getRequestCount() const
        {
            return _p->timeline->getRequestCount();
        }

        void TimelinePlayer::setRequestCount(size_t value)
        {
            _p->timeline->setRequestCount(value);
        }

        std::chrono::milliseconds TimelinePlayer::getRequestTimeout() const
        {
            return _p->timeline->getRequestTimeout();
        }

        void TimelinePlayer::setRequestTimeout(const std::chrono::milliseconds& value)
        {
            _p->timeline->setRequestTimeout(value);
        }

        void TimelinePlayer::setIOOptions(const avio::Options& value)
        {
            _p->timeline->setIOOptions(value);
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
            Frame frame;
            int frameCacheReadAhead = 0;
            int frameCacheReadBehind = 0;
            std::vector<otime::TimeRange> cachedFrames;
            {
                std::unique_lock<std::mutex> lock(p.threadData.mutex);
                p.threadData.currentTime = p.currentTime->get();
                frame = p.threadData.frame;
                frameCacheReadAhead = p.threadData.frameCacheReadAhead;
                frameCacheReadBehind = p.threadData.frameCacheReadBehind;
                cachedFrames = p.threadData.cachedFrames;
            }
            p.frame->setIfChanged(frame);
            size_t cachedFramesCount = 0;
            for (const auto& i : cachedFrames)
            {
                cachedFramesCount += i.duration().value();
            }
            p.frameCachePercentage->setIfChanged(
                cachedFramesCount / static_cast<float>(frameCacheReadAhead + frameCacheReadBehind) * 100.F);
            p.cachedFrames->setIfChanged(cachedFrames);
        }

        otime::RationalTime TimelinePlayer::Private::loopPlayback(const otime::RationalTime& time)
        {
            otime::RationalTime out = time;

            const auto range = inOutRange->get();
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

        void TimelinePlayer::Private::frameCacheUpdate(
            const otime::RationalTime& currentTime,
            const otime::TimeRange& inOutRange,
            uint16_t videoLayer,
            FrameCacheDirection frameCacheDirection,
            std::size_t frameCacheReadAhead,
            std::size_t frameCacheReadBehind)
        {
            // Get which frames should be cached.
            std::vector<otime::RationalTime> frames;
            const auto& duration = timeline->getDuration();
            auto time = currentTime;
            const auto& range = inOutRange;
            const size_t backwardsCount = FrameCacheDirection::Forward == frameCacheDirection ?
                frameCacheReadBehind :
                (frameCacheReadAhead - 1);
            for (std::size_t i = 0; i < backwardsCount; ++i)
            {
                time = loopTime(time - otime::RationalTime(1, duration.rate()), range);
            }
            for (std::size_t i = 0; i < frameCacheReadBehind + frameCacheReadAhead; ++i)
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
            std::list<std::shared_ptr<imaging::Image> > recycledImages;
            auto frameCacheIt = threadData.frameCache.begin();
            while (frameCacheIt != threadData.frameCache.end())
            {
                bool old = true;
                for (const auto& i : ranges)
                {
                    if (i.contains(frameCacheIt->second.time))
                    {
                        old = false;
                        break;
                    }
                }
                if (old)
                {
                    if (frame->get().time != frameCacheIt->second.time)
                    {
                        for (const auto& i : frameCacheIt->second.layers)
                        {
                            if (i.image)
                            {
                                i.image->setTags({});
                                recycledImages.push_back(i.image);
                            }
                        }
                    }
                    frameCacheIt = threadData.frameCache.erase(frameCacheIt);
                }
                else
                {
                    ++frameCacheIt;
                }
            }

            // Find uncached frames.
            std::vector<otime::RationalTime> uncached;
            for (const auto& i : frames)
            {
                const auto j = threadData.frameCache.find(i);
                if (j == threadData.frameCache.end())
                {
                    const auto k = threadData.frameRequests.find(i);
                    if (k == threadData.frameRequests.end())
                    {
                        uncached.push_back(i);
                    }
                }
            }

            // Get uncached frames.
            for (const auto& i : uncached)
            {
                std::shared_ptr<imaging::Image> recycledImage;
                if (!recycledImages.empty())
                {
                    recycledImage = recycledImages.front();
                    recycledImages.pop_front();
                }
                threadData.frameRequests[i] = timeline->getFrame(i, videoLayer, recycledImage);
            }
            auto framesIt = threadData.frameRequests.begin();
            while (framesIt != threadData.frameRequests.end())
            {
                if (framesIt->second.valid() &&
                    framesIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto frame = framesIt->second.get();
                    frame.time = framesIt->first;
                    threadData.frameCache[frame.time] = frame;
                    framesIt = threadData.frameRequests.erase(framesIt);
                }
                else
                {
                    ++framesIt;
                }
            }

            // Update cached frames.
            std::vector<otime::RationalTime> cachedFrames;
            for (const auto& i : threadData.frameCache)
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
