// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/TimelinePlayer.h>

#include <tlrCore/Error.h>
#include <tlrCore/File.h>
#include <tlrCore/IO.h>
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

        TLR_ENUM_IMPL(Loop, "Loop", "Once", "Ping-Pong");

        TLR_ENUM_IMPL(TimeAction,
            "Start",
            "End",
            "FramePrev",
            "FramePrevX10",
            "FramePrevX100",
            "FrameNext",
            "FrameNextX10",
            "FrameNextX100");

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

        math::BBox2f fitWindow(const imaging::Size& image, const imaging::Size& window)
        {
            math::BBox2f out;
            const float windowAspect = window.getAspect();
            const float imageAspect = image.getAspect();
            math::BBox2f bbox;
            if (windowAspect > imageAspect)
            {
                out = math::BBox2f(
                    window.w / 2.F - (window.h * imageAspect) / 2.F,
                    0.F,
                    window.h * imageAspect,
                    window.h);
            }
            else
            {
                out = math::BBox2f(
                    0.F,
                    window.h / 2.F - (window.w / imageAspect) / 2.F,
                    window.w,
                    window.w / imageAspect);
            }
            return out;
        }

        void TimelinePlayer::_init(const std::string& fileName)
        {
            // Create the timeline.
            _timeline = timeline::Timeline::create(fileName);

            // Create observers.
            _playback = observer::Value<Playback>::create(Playback::Stop);
            _loop = observer::Value<Loop>::create(Loop::Loop);
            _currentTime = observer::Value<otime::RationalTime>::create(_timeline->getGlobalStartTime());
            _inOutRange = observer::Value<otime::TimeRange>::create(
                otime::TimeRange(_timeline->getGlobalStartTime(), _timeline->getDuration()));
            _frame = observer::Value<io::VideoFrame>::create();
            _cachedFrames = observer::List<otime::TimeRange>::create();

            // Create a new thread.
            _threadData.currentTime = _currentTime->get();
            _threadData.inOutRange = _inOutRange->get();
            _threadData.running = true;
            _thread = std::thread(
                [this, fileName]
                {
                    while (_threadData.running)
                    {
                        otime::RationalTime currentTime = invalidTime;
                        otime::TimeRange inOutRange = invalidTimeRange;
                        bool clearVideoFrameRequests = false;
                        FrameCacheDirection frameCacheDirection = FrameCacheDirection::Forward;
                        std::size_t frameCacheReadAhead = 0;
                        std::size_t frameCacheReadBehind = 0;
                        {
                            std::unique_lock<std::mutex> lock(_threadData.mutex);
                            currentTime = _threadData.currentTime;
                            inOutRange = _threadData.inOutRange;
                            clearVideoFrameRequests = _threadData.clearVideoFrameRequests;
                            _threadData.clearVideoFrameRequests = false;
                            frameCacheDirection = _threadData.frameCacheDirection;
                            frameCacheReadAhead = _threadData.frameCacheReadAhead;
                            frameCacheReadBehind = _threadData.frameCacheReadBehind;
                        }

                        //! Clear video frame requests.
                        if (clearVideoFrameRequests)
                        {
                            _timeline->cancelRenders();
                            _threadData.videoFrameRequests.clear();
                        }

                        //! Tick the timeline.
                        _timeline->tick();

                        //! Update the frame cache.
                        _frameCacheUpdate(
                            currentTime,
                            inOutRange,
                            frameCacheDirection,
                            frameCacheReadAhead,
                            frameCacheReadBehind);

                        //! Update the video frame.
                        const auto i = _threadData.frameCache.find(currentTime);
                        if (i != _threadData.frameCache.end())
                        {
                            std::unique_lock<std::mutex> lock(_threadData.mutex);
                            _threadData.videoFrame = i->second;
                        }

                        time::sleep(std::chrono::microseconds(1000));
                    }
                });
        }

        TimelinePlayer::TimelinePlayer()
        {}

        TimelinePlayer::~TimelinePlayer()
        {
            _threadData.running = false;
            if (_thread.joinable())
            {
                _thread.join();
            }
        }

        std::shared_ptr<TimelinePlayer> TimelinePlayer::create(const std::string& fileName)
        {
            auto out = std::shared_ptr<TimelinePlayer>(new TimelinePlayer);
            out->_init(fileName);
            return out;
        }

        void TimelinePlayer::setPlayback(Playback value)
        {
            switch (_loop->get())
            {
            case Loop::Once:
                switch (value)
                {
                case Playback::Forward:
                    if (_currentTime->get() == _inOutRange->get().end_time_inclusive())
                    {
                        seek(_inOutRange->get().start_time());
                    }
                    break;
                case Playback::Reverse:
                    if (_currentTime->get() == _inOutRange->get().start_time())
                    {
                        seek(_inOutRange->get().end_time_inclusive());
                    }
                    break;
                }
                break;
            case Loop::PingPong:
                switch (value)
                {
                case Playback::Forward:
                    if (_currentTime->get() == _inOutRange->get().end_time_inclusive())
                    {
                        value = Playback::Reverse;
                    }
                    break;
                case Playback::Reverse:
                    if (_currentTime->get() == _inOutRange->get().start_time())
                    {
                        value = Playback::Forward;
                    }
                    break;
                }
                break;
            default: break;
            }
            if (_playback->setIfChanged(value))
            {
                if (value != Playback::Stop)
                {
                    _startTime = std::chrono::steady_clock::now();
                    _playbackStartTime = _currentTime->get();

                    std::unique_lock<std::mutex> lock(_threadData.mutex);
                    _threadData.frameCacheDirection = Playback::Forward == value ? FrameCacheDirection::Forward : FrameCacheDirection::Reverse;
                }
            }
        }

        void TimelinePlayer::setLoop(Loop value)
        {
            _loop->setIfChanged(value);
        }

        void TimelinePlayer::seek(const otime::RationalTime& time)
        {
            // Loop the time.
            auto range = otime::TimeRange(_timeline->getGlobalStartTime(), _timeline->getDuration());
            const auto tmp = loopTime(time, range);

            if (_currentTime->setIfChanged(tmp))
            {
                //std::cout << "! " << tmp << std::endl;

                // Update playback.
                if (_playback->get() != Playback::Stop)
                {
                    _startTime = std::chrono::steady_clock::now();
                    _playbackStartTime = _currentTime->get();
                }

                {
                    std::unique_lock<std::mutex> lock(_threadData.mutex);
                    _threadData.currentTime = tmp;
                    _threadData.clearVideoFrameRequests = true;
                }
            }
        }

        void TimelinePlayer::timeAction(TimeAction time)
        {
            setPlayback(timeline::Playback::Stop);
            const auto& duration = _timeline->getDuration();
            const auto& currentTime = _currentTime->get();
            switch (time)
            {
            case TimeAction::Start:
                seek(_inOutRange->get().start_time());
                break;
            case TimeAction::End:
                seek(_inOutRange->get().end_time_inclusive());
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

        void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
        {
            if (_inOutRange->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(_threadData.mutex);
                _threadData.inOutRange = value;
            }
        }

        void TimelinePlayer::setInPoint()
        {
            setInOutRange(otime::TimeRange::range_from_start_end_time(
                _currentTime->get(),
                _inOutRange->get().end_time_exclusive()));
        }

        void TimelinePlayer::resetInPoint()
        {
            setInOutRange(otime::TimeRange::range_from_start_end_time(
                _timeline->getGlobalStartTime(),
                _inOutRange->get().end_time_exclusive()));
        }

        void TimelinePlayer::setOutPoint()
        {
            setInOutRange(otime::TimeRange::range_from_start_end_time_inclusive(
                _inOutRange->get().start_time(),
                _currentTime->get()));
        }

        void TimelinePlayer::resetOutPoint()
        {
            setInOutRange(otime::TimeRange(
                _inOutRange->get().start_time(),
                _timeline->getDuration()));
        }

        int TimelinePlayer::getFrameCacheReadAhead()
        {
            std::unique_lock<std::mutex> lock(_threadData.mutex);
            return _threadData.frameCacheReadAhead;
        }

        int TimelinePlayer::getFrameCacheReadBehind()
        {
            std::unique_lock<std::mutex> lock(_threadData.mutex);
            return _threadData.frameCacheReadBehind;
        }

        void TimelinePlayer::setFrameCacheReadAhead(int value)
        {
            std::unique_lock<std::mutex> lock(_threadData.mutex);
            _threadData.frameCacheReadAhead = value;
        }

        void TimelinePlayer::setFrameCacheReadBehind(int value)
        {
            std::unique_lock<std::mutex> lock(_threadData.mutex);
            _threadData.frameCacheReadBehind = value;
        }

        void TimelinePlayer::tick()
        {
            // Calculate the current time.
            otio::ErrorStatus errorStatus;
            const auto playback = _playback->get();
            if (playback != Playback::Stop)
            {
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - _startTime;
                const auto& duration = _timeline->getDuration();
                auto currentTime = _loopPlayback(_playbackStartTime +
                    otime::RationalTime(floor(diff.count() * duration.rate() * (Playback::Forward == playback ? 1.0 : -1.0)), duration.rate()));
                if (_currentTime->setIfChanged(currentTime))
                {
                    //std::cout << "! " << _currentTime->get() << std::endl;
                }
            }

            // Sync with the thread.
            io::VideoFrame videoFrame;
            std::vector<otime::TimeRange> cachedFrames;
            {
                std::unique_lock<std::mutex> lock(_threadData.mutex);
                _threadData.currentTime = _currentTime->get();
                videoFrame = _threadData.videoFrame;
                cachedFrames = _threadData.cachedFrames;
            }
            _frame->setIfChanged(videoFrame);
            _cachedFrames->setIfChanged(cachedFrames);
        }

        otime::RationalTime TimelinePlayer::_loopPlayback(const otime::RationalTime& time)
        {
            otime::RationalTime out = time;

            const auto range = _inOutRange->get();
            switch (_loop->get())
            {
            case Loop::Loop:
            {
                const auto tmp = loopTime(out, range);
                if (tmp != out)
                {
                    out = tmp;
                    _startTime = std::chrono::steady_clock::now();
                    _playbackStartTime = tmp;
                }
                break;
            }
            case Loop::Once:
                if (out < range.start_time())
                {
                    out = range.start_time();
                    _playback->setIfChanged(Playback::Stop);
                }
                else if (out > range.end_time_inclusive())
                {
                    out = range.end_time_inclusive();
                    _playback->setIfChanged(Playback::Stop);
                }
                break;
            case Loop::PingPong:
            {
                const auto playback = _playback->get();
                if (out < range.start_time() && Playback::Reverse == playback)
                {
                    out = range.start_time();
                    _playback->setIfChanged(Playback::Forward);
                    _startTime = std::chrono::steady_clock::now();
                    _playbackStartTime = out;
                }
                else if (out > range.end_time_inclusive() && Playback::Forward == playback)
                {
                    out = range.end_time_inclusive();
                    _playback->setIfChanged(Playback::Reverse);
                    _startTime = std::chrono::steady_clock::now();
                    _playbackStartTime = out;
                }
                break;
            }
            default: break;
            }

            return out;
        }

        void TimelinePlayer::_frameCacheUpdate(
            const otime::RationalTime& currentTime,
            const otime::TimeRange& inOutRange,
            FrameCacheDirection frameCacheDirection,
            std::size_t frameCacheReadAhead,
            std::size_t frameCacheReadBehind)
        {
            // Get which frames should be cached.
            std::vector<otime::RationalTime> frames;
            const auto& duration = _timeline->getDuration();
            auto time = currentTime;
            const auto& range = inOutRange;
            for (std::size_t i = 0; i < (FrameCacheDirection::Forward == frameCacheDirection ? frameCacheReadBehind : frameCacheReadAhead); ++i)
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
            _timeline->setActiveRanges(ranges);

            // Remove old frames from the cache.
            std::list<std::shared_ptr<imaging::Image> > removed;
            auto frameCacheIt = _threadData.frameCache.begin();
            while (frameCacheIt != _threadData.frameCache.end())
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
                    removed.push_back(frameCacheIt->second.image);
                    frameCacheIt = _threadData.frameCache.erase(frameCacheIt);
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
                const auto j = _threadData.frameCache.find(i);
                if (j == _threadData.frameCache.end())
                {
                    const auto k = _threadData.videoFrameRequests.find(i);
                    if (k == _threadData.videoFrameRequests.end())
                    {
                        uncached.push_back(i);
                    }
                }
            }

            // Get uncached frames.
            for (const auto& i : uncached)
            {
                _threadData.videoFrameRequests[i] = _timeline->render(i);
            }
            auto videoFramesIt = _threadData.videoFrameRequests.begin();
            while (videoFramesIt != _threadData.videoFrameRequests.end())
            {
                if (videoFramesIt->second.valid() &&
                    videoFramesIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto frame = videoFramesIt->second.get();
                    frame.time = videoFramesIt->first;
                    _threadData.frameCache[frame.time] = frame;
                    videoFramesIt = _threadData.videoFrameRequests.erase(videoFramesIt);
                }
                else
                {
                    ++videoFramesIt;
                }
            }

            // Update cached frames.
            std::vector<otime::RationalTime> cachedFrames;
            for (const auto& i : _threadData.frameCache)
            {
                cachedFrames.push_back(i.second.time);
            }
            {
                std::unique_lock<std::mutex> lock(_threadData.mutex);
                _threadData.cachedFrames = toRanges(cachedFrames);
            }
        }
    }

    TLR_ENUM_SERIALIZE_IMPL(timeline, Playback);
    TLR_ENUM_SERIALIZE_IMPL(timeline, Loop);
    TLR_ENUM_SERIALIZE_IMPL(timeline, TimeAction);
}
