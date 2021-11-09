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
        //! Number of frames in the audio buffer.
        const size_t bufferFrameCount = 256;
    
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
        otime::RationalTime loopTime(const otime::RationalTime&, const otime::TimeRange&, bool* loop = nullptr);

        //! Timeline player.
        class TimelinePlayer : public std::enable_shared_from_this<TimelinePlayer>
        {
            TLR_NON_COPYABLE(TimelinePlayer);

        protected:
            void _init(
                const file::Path&,
                const std::shared_ptr<core::Context>&,
                const Options&);
            TimelinePlayer();

        public:
            ~TimelinePlayer();

            //! Create a new timeline player.
            static std::shared_ptr<TimelinePlayer> create(
                const file::Path&,
                const std::shared_ptr<core::Context>&,
                const Options& = Options());

            //! Get the context.
            const std::weak_ptr<core::Context>& getContext() const;

            //! Get the timeline.
            const otio::SerializableObject::Retainer<otio::Timeline>& getTimeline() const;
            
            //! Get the path.
            const file::Path& getPath() const;

            //! Get the options.
            const Options& getOptions() const;

            //! \name Information
            ///@{

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the global start time.
            const otime::RationalTime& getGlobalStartTime() const;

            //! Get the A/V information. This information is retreived from
            //! the first clip in the timeline.
            const avio::Info& getAVInfo() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the default playback speed.
            double getDefaultSpeed() const;

            //! Observe the playback speed.
            std::shared_ptr<observer::IValue<double> > observeSpeed() const;

            //! Set the playback speed.
            void setSpeed(double);

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

            //! \name Video
            ///@{

            //! Observer the current video layer.
            std::shared_ptr<observer::IValue<uint16_t> > observeVideoLayer() const;

            //! Set the current video layer.
            void setVideoLayer(uint16_t);

            //! Observe the current video data.
            std::shared_ptr<observer::IValue<VideoData> > observeVideo() const;

            //! Get the cache read ahead.
            size_t getCacheReadAhead();

            //! Set the cache read ahead.
            void setCacheReadAhead(size_t);

            //! Get the cache read behind.
            size_t getCacheReadBehind();

            //! Set the cache read behind.
            void setCacheReadBehind(size_t);

            //! Observe the cache percentage.
            std::shared_ptr<observer::IValue<float> > observeCachePercentage() const;

            //! Observe the cached video frames.
            std::shared_ptr<observer::IList<otime::TimeRange> > observeCachedVideoFrames() const;

            ///@}

            //! \name Audio
            ///@{

            //! Observer the audio volume.
            std::shared_ptr<observer::IValue<float> > observeVolume() const;

            //! Set the audio volume.
            void setVolume(float);

            //! Observer the audio mute.
            std::shared_ptr<observer::IValue<bool> > observeMute() const;

            //! Set the audio mute.
            void setMute(bool);

            //! Observe the cached audio frames.
            std::shared_ptr<observer::IList<otime::TimeRange> > observeCachedAudioFrames() const;

            ///@}

            //! Tick the timeline.
            void tick();

        private:
            TLR_PRIVATE();
        };
    }
}
