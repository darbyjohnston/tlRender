// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <tlCore/ListObserver.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace timeline
    {
        //! Timer modes.
        enum class TimerMode
        {
            System,
            Audio,

            Count,
            First = System
        };
        TLRENDER_ENUM(TimerMode);
        TLRENDER_ENUM_SERIALIZE(TimerMode);

        //! Audio buffer frame counts.
        enum class AudioBufferFrameCount
        {
            _16,
            _32,
            _64,
            _128,
            _256,
            _512,
            _1024,

            Count,
            First = _16
        };
        TLRENDER_ENUM(AudioBufferFrameCount);
        TLRENDER_ENUM_SERIALIZE(AudioBufferFrameCount);

        //! Get the audio buffer frame count.
        size_t getAudioBufferFrameCount(AudioBufferFrameCount);

        //! Timeline player options.
        struct PlayerOptions
        {
            otime::RationalTime cacheReadAhead = otime::RationalTime(4.0, 1.0);
            otime::RationalTime cacheReadBehind = otime::RationalTime(0.5, 1.0);

            TimerMode timerMode = TimerMode::System;

            AudioBufferFrameCount audioBufferFrameCount = AudioBufferFrameCount::_256;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout = std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::microseconds sleepTimeout = std::chrono::microseconds(1000);

            bool operator == (const PlayerOptions&) const;
            bool operator != (const PlayerOptions&) const;
        };

        //! Playback modes.
        enum class Playback
        {
            Stop,
            Forward,
            Reverse,

            Count,
            First = Stop
        };
        TLRENDER_ENUM(Playback);
        TLRENDER_ENUM_SERIALIZE(Playback);

        //! Playback loop modes.
        enum class Loop
        {
            Loop,
            Once,
            PingPong,

            Count,
            First = Loop
        };
        TLRENDER_ENUM(Loop);
        TLRENDER_ENUM_SERIALIZE(Loop);

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
        TLRENDER_ENUM(TimeAction);
        TLRENDER_ENUM_SERIALIZE(TimeAction);

        //! Loop a time.
        otime::RationalTime loop(
            const otime::RationalTime&,
            const otime::TimeRange&,
            bool* looped = nullptr);

        //! Loop a range.
        std::vector<otime::TimeRange> loop(
            const otime::TimeRange&,
            const otime::TimeRange&);

        //! Timeline player.
        class TimelinePlayer : public std::enable_shared_from_this<TimelinePlayer>
        {
            TLRENDER_NON_COPYABLE(TimelinePlayer);

        protected:
            void _init(
                const std::shared_ptr<Timeline>&,
                const std::shared_ptr<core::Context>&,
                const PlayerOptions&);
            TimelinePlayer();

        public:
            ~TimelinePlayer();

            //! Create a new timeline player.
            static std::shared_ptr<TimelinePlayer> create(
                const std::shared_ptr<Timeline>&,
                const std::shared_ptr<core::Context>&,
                const PlayerOptions & = PlayerOptions());

            //! Get the context.
            const std::weak_ptr<core::Context>& getContext() const;

            //! Get the timeline.
            const std::shared_ptr<Timeline>& getTimeline() const;

            //! Get the path.
            const core::file::Path& getPath() const;

            //! Get the audio path.
            const core::file::Path& getAudioPath() const;

            //! Get the timeline player options.
            const PlayerOptions& getPlayerOptions() const;

            //! Get the timeline options.
            const Options& getOptions() const;

            //! \name Information
            ///@{

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the global start time.
            const otime::RationalTime& getGlobalStartTime() const;

            //! Get the I/O information. This information is retreived from
            //! the first clip in the timeline.
            const io::Info& getIOInfo() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the default playback speed.
            double getDefaultSpeed() const;

            //! Observe the playback speed.
            std::shared_ptr<core::observer::IValue<double> > observeSpeed() const;

            //! Set the playback speed.
            void setSpeed(double);

            //! Observe the playback mode.
            std::shared_ptr<core::observer::IValue<Playback> > observePlayback() const;

            //! Set the playback mode.
            void setPlayback(Playback);

            //! Observe the playback loop mode.
            std::shared_ptr<core::observer::IValue<Loop> > observeLoop() const;

            //! Set the playback loop mode.
            void setLoop(Loop);

            ///@}

            //! \name Time
            ///@{

            //! Observe the current time.
            std::shared_ptr<core::observer::IValue<otime::RationalTime> > observeCurrentTime() const;

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

            //! Use the time from a separate timeline player.
            void setExternalTime(const std::shared_ptr<TimelinePlayer>&);

            ///@}

            //! \name In/Out Points
            ///@{

            //! Observe the in/out points range.
            std::shared_ptr<core::observer::IValue<otime::TimeRange> > observeInOutRange() const;

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

            //! Observe the current video layer.
            std::shared_ptr<core::observer::IValue<uint16_t> > observeVideoLayer() const;

            //! Set the current video layer.
            void setVideoLayer(uint16_t);

            //! Observe the current video data.
            std::shared_ptr<core::observer::IValue<VideoData> > observeVideo() const;

            ///@}

            //! \name Audio
            ///@{

            //! Observe the audio volume.
            std::shared_ptr<core::observer::IValue<float> > observeVolume() const;

            //! Set the audio volume.
            void setVolume(float);

            //! Increase the audio volume
            void increaseVolume();

            //! Decrease the audio volume
            void decreaseVolume();

            //! Observe the audio mute.
            std::shared_ptr<core::observer::IValue<bool> > observeMute() const;

            //! Set the audio mute.
            void setMute(bool);

            //! Observe the audio sync offset (in seconds).
            std::shared_ptr<core::observer::IValue<double> > observeAudioOffset() const;

            //! Set the audio sync offset (in seconds).
            void setAudioOffset(double);

            ///@}

            //! \name Cache
            ///@{

            //! Observe the cache read ahead.
            std::shared_ptr<core::observer::IValue<otime::RationalTime> > observeCacheReadAhead() const;

            //! Set the cache read ahead.
            void setCacheReadAhead(const otime::RationalTime&);

            //! Observe the cache read behind.
            std::shared_ptr<core::observer::IValue<otime::RationalTime> > observeCacheReadBehind() const;

            //! Set the cache read behind.
            void setCacheReadBehind(const otime::RationalTime&);

            //! Observe the cache percentage.
            std::shared_ptr<core::observer::IValue<float> > observeCachePercentage() const;

            //! Observe the cached video.
            std::shared_ptr<core::observer::IList<otime::TimeRange> > observeCachedVideoFrames() const;

            //! Observe the cached audio.
            std::shared_ptr<core::observer::IList<otime::TimeRange> > observeCachedAudioFrames() const;

            ///@}

            //! Tick the timeline.
            void tick();

        private:
            TLRENDER_PRIVATE();
        };
    }
}

#include <tlTimeline/TimelinePlayerInline.h>
