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
        TLR_ENUM_SERIALIZE(Playback);

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
        TLR_ENUM_SERIALIZE(Loop);

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
        TLR_ENUM_SERIALIZE(TimeAction);

        //! Loop time.
        otime::RationalTime loopTime(const otime::RationalTime&, const otime::TimeRange&);

        //! Timeline player.
        class TimelinePlayer : public std::enable_shared_from_this<TimelinePlayer>
        {
            TLR_NON_COPYABLE(TimelinePlayer);

        protected:
            void _init(
                const file::Path&,
                const std::shared_ptr<core::Context>&);
            TimelinePlayer();

        public:
            ~TimelinePlayer();

            //! Create a new timeline player.
            static std::shared_ptr<TimelinePlayer> create(
                const file::Path&,
                const std::shared_ptr<core::Context>&);

            //! Get the context.
            const std::weak_ptr<core::Context>& getContext() const;

            //! Get the path.
            const file::Path& getPath() const;

            //! \name Information
            ///@{

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the global start time.
            const otime::RationalTime& getGlobalStartTime() const;

            //! Get the image info.
            const imaging::Info& getImageInfo() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the default playback speed.
            float getDefaultSpeed() const;

            //! Observe the playback speed.
            std::shared_ptr<observer::IValue<float> > observeSpeed() const;

            //! Set the playback speed.
            void setSpeed(float);

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

            //! Observe the frame cache percentage.
            std::shared_ptr<observer::IValue<float> > observeFrameCachePercentage() const;

            //! Observe the cached frames.
            std::shared_ptr<observer::IList<otime::TimeRange> > observeCachedFrames() const;

            ///@}

            //! \name Options
            ///@{

            //! Get the number of frame requests.
            size_t getRequestCount() const;

            //! Set the number of frame requests.
            void setRequestCount(size_t);

            //! Get the frame request timeout.
            std::chrono::milliseconds getRequestTimeout() const;

            //! Set the frame request timeout.
            void setRequestTimeout(const std::chrono::milliseconds&);

            ///@}

            //! Tick the timeline.
            void tick();

        private:
            TLR_PRIVATE();
        };
    }
}
