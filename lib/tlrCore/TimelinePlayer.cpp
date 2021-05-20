// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/TimelinePlayer.h>

#include <tlrCore/Error.h>
#include <tlrCore/File.h>
#include <tlrCore/IO.h>
#include <tlrCore/String.h>

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
        TLR_ENUM_VECTOR_IMPL(Playback);
        TLR_ENUM_LABEL_IMPL(Playback, "Stop", "Forward", "Reverse");

        TLR_ENUM_VECTOR_IMPL(Loop);
        TLR_ENUM_LABEL_IMPL(Loop, "Loop", "Once", "Ping-Pong");

        TLR_ENUM_VECTOR_IMPL(TimeAction);
        TLR_ENUM_LABEL_IMPL(TimeAction,
            "Start",
            "End",
            "FramePrev",
            "FramePrevX10",
            "FramePrevX100",
            "FrameNext",
            "FrameNextX10",
            "FrameNextX100",
            "ClipPrev",
            "ClipNext");

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

        void TimelinePlayer::_init(const std::shared_ptr<Timeline>& timeline)
        {
            _timeline = timeline;

            // Create observers.
            _playback = Observer::ValueSubject<Playback>::create(Playback::Stop);
            _loop = Observer::ValueSubject<Loop>::create(Loop::Loop);
            const auto& globalStartTime = _timeline->getGlobalStartTime();
            _currentTime = Observer::ValueSubject<otime::RationalTime>::create(globalStartTime);
            const auto& duration = _timeline->getDuration();
            _inOutRange = Observer::ValueSubject<otime::TimeRange>::create(otime::TimeRange(globalStartTime, duration));
            _frame = Observer::ValueSubject<io::VideoFrame>::create();
            _cachedFrames = Observer::ListSubject<otime::TimeRange>::create();
        }

        TimelinePlayer::TimelinePlayer()
        {}

        TimelinePlayer::~TimelinePlayer()
        {}

        std::shared_ptr<TimelinePlayer> TimelinePlayer::create(const std::shared_ptr<Timeline>& timeline)
        {
            auto out = std::shared_ptr<TimelinePlayer>(new TimelinePlayer);
            out->_init(timeline);
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
            default:
                break;
            }
            if (_playback->setIfChanged(value))
            {
                if (value != Playback::Stop)
                {
                    _startTime = std::chrono::steady_clock::now();
                    _playbackStartTime = _currentTime->get();
                    _frameCacheDirection = Playback::Forward == value ? FrameCacheDirection::Forward : FrameCacheDirection::Reverse;
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
                // Update playback.
                if (_playback->get() != Playback::Stop)
                {
                    _startTime = std::chrono::steady_clock::now();
                    _playbackStartTime = _currentTime->get();
                }

                // Cancel renders.
                _timeline->cancelRenders();
                _videoFrameRequests.clear();
            }
        }

        void TimelinePlayer::timeAction(TimeAction time)
        {
            setPlayback(timeline::Playback::Stop);
            const auto& currentTime = _currentTime->get();
            const auto& duration = _timeline->getDuration();
            const auto& clipRanges = _timeline->getClipRanges();
            const std::size_t rangeSize = clipRanges.size();
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
            case TimeAction::ClipPrev:
                for (std::size_t i = 0; i < rangeSize; ++i)
                {
                    if (clipRanges[i].contains(currentTime))
                    {
                        if (i > 0)
                        {
                            seek(clipRanges[i - 1].start_time());
                        }
                        else
                        {
                            seek(clipRanges[rangeSize - 1].start_time());
                        }
                        break;
                    }
                }
                break;
            case TimeAction::ClipNext:
                for (std::size_t i = 0; i < rangeSize; ++i)
                {
                    if (clipRanges[i].contains(currentTime))
                    {
                        if ((i + 1) < rangeSize)
                        {
                            seek(clipRanges[i + 1].start_time());
                        }
                        else
                        {
                            seek(clipRanges[0].start_time());
                        }
                        break;
                    }
                }
                break;
            default:
                break;
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

        void TimelinePlayer::clipPrev()
        {
            timeAction(TimeAction::ClipPrev);
        }

        void TimelinePlayer::clipNext()
        {
            timeAction(TimeAction::ClipNext);
        }

        void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
        {
            _inOutRange->setIfChanged(value);
        }

        void TimelinePlayer::setInPoint()
        {
            _inOutRange->setIfChanged(otime::TimeRange::range_from_start_end_time(
                _currentTime->get(),
                _inOutRange->get().end_time_exclusive()));
        }

        void TimelinePlayer::resetInPoint()
        {
            _inOutRange->setIfChanged(otime::TimeRange::range_from_start_end_time(
                _timeline->getGlobalStartTime(),
                _inOutRange->get().end_time_exclusive()));
        }

        void TimelinePlayer::setOutPoint()
        {
            _inOutRange->setIfChanged(otime::TimeRange::range_from_start_end_time_inclusive(
                _inOutRange->get().start_time(),
                _currentTime->get()));
        }

        void TimelinePlayer::resetOutPoint()
        {
            _inOutRange->setIfChanged(otime::TimeRange(
                _inOutRange->get().start_time(),
                _timeline->getDuration()));
        }

        void TimelinePlayer::setFrameCacheReadAhead(int value)
        {
            _frameCacheReadAhead = value;
        }

        void TimelinePlayer::setFrameCacheReadBehind(int value)
        {
            _frameCacheReadBehind = value;
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
                auto currentTime = _playbackStartTime +
                    otime::RationalTime(floor(diff.count() * duration.rate() * (Playback::Forward == playback ? 1.0 : -1.0)), duration.rate());
                _currentTime->setIfChanged(_loopPlayback(currentTime));
            }

            //! Tick the timeline.
            _timeline->tick();

            //! Update the frame cache.
            _frameCacheUpdate();

            // Update the current frame.
            const auto i = _frameCache.find(_currentTime->get());
            _frame->setIfChanged(i != _frameCache.end() ? i->second : io::VideoFrame());
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
            default:
                break;
            }

            return out;
        }

        void TimelinePlayer::_frameCacheUpdate()
        {
            // Get which frames should be cached.
            std::vector<otime::RationalTime> frames;
            auto time = _currentTime->get();
            const auto& range = _inOutRange->get();
            const auto& duration = _timeline->getDuration();
            for (std::size_t i = 0; i < (FrameCacheDirection::Forward == _frameCacheDirection ? _frameCacheReadBehind : _frameCacheReadAhead); ++i)
            {
                time = loopTime(time - otime::RationalTime(1, duration.rate()), range);
            }
            for (std::size_t i = 0; i < _frameCacheReadBehind + _frameCacheReadAhead; ++i)
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
            auto frameCacheIt = _frameCache.begin();
            while (frameCacheIt != _frameCache.end())
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
                    frameCacheIt = _frameCache.erase(frameCacheIt);
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
                const auto j = _frameCache.find(i);
                if (j == _frameCache.end())
                {
                    const auto k = _videoFrameRequests.find(i);
                    if (k == _videoFrameRequests.end())
                    {
                        uncached.push_back(i);
                    }
                }
            }

            // Get uncached frames.
            for (const auto& i : uncached)
            {
                _videoFrameRequests[i] = _timeline->render(i);
            }
            auto videoFramesIt = _videoFrameRequests.begin();
            while (videoFramesIt != _videoFrameRequests.end())
            {
                if (videoFramesIt->second.valid() &&
                    videoFramesIt->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto frame = videoFramesIt->second.get();
                    frame.time = videoFramesIt->first;
                    _frameCache[frame.time] = frame;
                    videoFramesIt = _videoFrameRequests.erase(videoFramesIt);
                }
                else
                {
                    ++videoFramesIt;
                }
            }

            // Update cached frames.
            std::vector<otime::RationalTime> cachedFrames;
            for (const auto& i : _frameCache)
            {
                cachedFrames.push_back(i.second.time);
            }
            _cachedFrames->setIfChanged(toRanges(cachedFrames));
        }
    }

    TLR_ENUM_SERIALIZE_IMPL(timeline, Playback);
    TLR_ENUM_SERIALIZE_IMPL(timeline, Loop);
}
