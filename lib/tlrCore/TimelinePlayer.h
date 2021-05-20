// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/BBox.h>
#include <tlrCore/ListObserver.h>
#include <tlrCore/Timeline.h>
#include <tlrCore/ValueObserver.h>

namespace tlr
{
    //! Timelines.
    namespace timeline
    {
        //! Playback modes.
        enum class Playback
        {
            Stop,
            Forward,
            Reverse,

            Count,
            First = Stop
        };
        TLR_ENUM_VECTOR(Playback);
        TLR_ENUM_LABEL(Playback);

        //! Playback loop modes.
        enum class Loop
        {
            Loop,
            Once,
            PingPong,

            Count,
            First = Loop
        };
        TLR_ENUM_VECTOR(Loop);
        TLR_ENUM_LABEL(Loop);

        //! Time actions.
        enum class TimeAction
        {
            Start,
            End,
            FramePrev,
            FramePrevX10,
            FramePrevX100,
            FrameNext,
            FrameNextX10,
            FrameNextX100,
            ClipPrev,
            ClipNext,

            Count,
            First = Start
        };
        TLR_ENUM_VECTOR(TimeAction);
        TLR_ENUM_LABEL(TimeAction);

        //! Loop time.
        otime::RationalTime loopTime(const otime::RationalTime&, const otime::TimeRange&);

        //! Fit an image within a window.
        math::BBox2f fitWindow(const imaging::Size& image, const imaging::Size& window);

        //! Timeline player.
        class TimelinePlayer : public std::enable_shared_from_this<TimelinePlayer>
        {
            TLR_NON_COPYABLE(TimelinePlayer);

        protected:
            void _init(const std::shared_ptr<Timeline>&);
            TimelinePlayer();

        public:
            ~TimelinePlayer();

            //! Create a new timeline player.
            static std::shared_ptr<TimelinePlayer> create(const std::shared_ptr<Timeline>&);

            //! \name Information
            ///@{

            //! Get the file name.
            const std::string& getFileName() const;

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the global start time.
            const otime::RationalTime& getGlobalStartTime() const;

            //! Get the image info.
            const imaging::Info& getImageInfo() const;

            //! Get the clip time ranges.
            std::vector<otime::TimeRange> getClipRanges() const;

            ///@}

            //! \name Playback
            ///@{

            //! Observe the playback mode.
            std::shared_ptr<Observer::IValueSubject<Playback> > observePlayback() const;

            //! Set the playback mode.
            void setPlayback(Playback);

            //! Observe the playback loop mode.
            std::shared_ptr<Observer::IValueSubject<Loop> > observeLoop() const;

            //! Set the playback loop mode.
            void setLoop(Loop);

            ///@}

            //! \name Time
            ///@{

            //! Observe the current time.
            std::shared_ptr<Observer::IValueSubject<otime::RationalTime> > observeCurrentTime() const;

            //! Seek to the given time.
            void seek(const otime::RationalTime&);

            //! Time action.
            void timeAction(TimeAction);

            //! Go to the start time.
            void start();

            //! Go to the end time.
            void end();

            //! Go to the previous frame.
            void framePrev();

            //! Go to the next frame.
            void frameNext();

            //! Go to the previous clip.
            void clipPrev();

            //! Go to the next clip.
            void clipNext();

            ///@}

            //! \name In/Out Points
            ///@{

            //! Observe the in/out points range.
            std::shared_ptr<Observer::IValueSubject<otime::TimeRange> > observeInOutRange() const;

            //! Set the in/out points range.
            void setInOutRange(const otime::TimeRange&);

            //! Set the in point to the current time.
            void setInPoint();

            //! Reset the in point
            void resetInPoint();

            //! Set the out point to the current time.
            void setOutPoint();

            //! Reset the out point
            void resetOutPoint();

            ///@}

            //! \name Frames
            ///@{

            //! Observe the current frame.
            std::shared_ptr<Observer::IValueSubject<io::VideoFrame> > observeFrame() const;

            //! Get the frame cache read ahead.
            int getFrameCacheReadAhead() const;

            //! Set the frame cache read ahead.
            void setFrameCacheReadAhead(int);

            //! Get the frame cache read behind.
            int getFrameCacheReadBehind() const;

            //! Set the frame cache read behind.
            void setFrameCacheReadBehind(int);

            //! Observe the cached frames.
            std::shared_ptr<Observer::IListSubject<otime::TimeRange> > observeCachedFrames() const;

            ///@}

            //! Tick the timeline.
            void tick();

        private:
            otime::RationalTime _loopPlayback(const otime::RationalTime&);
            void _frameCacheUpdate();

            std::shared_ptr<Timeline> _timeline;
            std::map<otime::RationalTime, std::future<io::VideoFrame> > _videoFrameRequests;
            std::shared_ptr<Observer::ValueSubject<Playback> > _playback;
            std::shared_ptr<Observer::ValueSubject<Loop> > _loop;
            std::shared_ptr<Observer::ValueSubject<otime::RationalTime> > _currentTime;
            std::shared_ptr<Observer::ValueSubject<otime::TimeRange> > _inOutRange;
            std::shared_ptr<Observer::ValueSubject<io::VideoFrame> > _frame;
            std::shared_ptr<Observer::ListSubject<otime::TimeRange> > _cachedFrames;
            std::chrono::steady_clock::time_point _startTime;
            otime::RationalTime _playbackStartTime;
            std::map<otime::RationalTime, io::VideoFrame> _frameCache;
            enum class FrameCacheDirection
            {
                Forward,
                Reverse
            };
            FrameCacheDirection _frameCacheDirection = FrameCacheDirection::Forward;
            std::size_t _frameCacheReadAhead = 100;
            std::size_t _frameCacheReadBehind = 10;
        };
    }

    TLR_ENUM_SERIALIZE(timeline::Playback);
    TLR_ENUM_SERIALIZE(timeline::Loop);
    TLR_ENUM_SERIALIZE(timeline::TimeAction);
}

#include <tlrCore/TimelinePlayerInline.h>
