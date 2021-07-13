// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

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
        TLR_ENUM(Playback);

        //! Playback loop modes.
        enum class Loop
        {
            Loop,
            Once,
            PingPong,

            Count,
            First = Loop
        };
        TLR_ENUM(Loop);

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

            Count,
            First = Start
        };
        TLR_ENUM(TimeAction);

        //! Loop time.
        otime::RationalTime loopTime(const otime::RationalTime&, const otime::TimeRange&);

        //! Timeline player.
        class TimelinePlayer : public std::enable_shared_from_this<TimelinePlayer>
        {
            TLR_NON_COPYABLE(TimelinePlayer);

        protected:
            void _init(const std::string& fileName);
            TimelinePlayer();

        public:
            ~TimelinePlayer();

            //! Create a new timeline player.
            static std::shared_ptr<TimelinePlayer> create(const std::string& fileName);

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

            ///@}

            //! \name Playback
            ///@{

            //! Observe the playback mode.
            std::shared_ptr<observer::IValue<Playback> > observePlayback() const;

            //! Set the playback mode.
            void setPlayback(Playback);

            //! Observe the playback loop mode.
            std::shared_ptr<observer::IValue<Loop> > observeLoop() const;

            //! Set the playback loop mode.
            void setLoop(Loop);

            ///@}

            //! \name Time
            ///@{

            //! Observe the current time.
            std::shared_ptr<observer::IValue<otime::RationalTime> > observeCurrentTime() const;

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

            ///@}

            //! \name In/Out Points
            ///@{

            //! Observe the in/out points range.
            std::shared_ptr<observer::IValue<otime::TimeRange> > observeInOutRange() const;

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
            std::shared_ptr<observer::IValue<Frame> > observeFrame() const;

            //! Get the frame cache read ahead.
            int getFrameCacheReadAhead();

            //! Set the frame cache read ahead.
            void setFrameCacheReadAhead(int);

            //! Get the frame cache read behind.
            int getFrameCacheReadBehind();

            //! Set the frame cache read behind.
            void setFrameCacheReadBehind(int);

            //! Observe the cached frames.
            std::shared_ptr<observer::IList<otime::TimeRange> > observeCachedFrames() const;

            ///@}

            //! Tick the timeline.
            void tick();

        private:
            TLR_PRIVATE();
        };
    }

    TLR_ENUM_SERIALIZE(timeline::Playback);
    TLR_ENUM_SERIALIZE(timeline::Loop);
    TLR_ENUM_SERIALIZE(timeline::TimeAction);
}
