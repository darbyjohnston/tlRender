// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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

        //! Timeline player cache options.
        struct PlayerCacheOptions
        {
            //! Cache read ahead.
            otime::RationalTime readAhead = otime::RationalTime(4.0, 1.0);

            //! Cache read behind.
            otime::RationalTime readBehind = otime::RationalTime(0.5, 1.0);

            bool operator == (const PlayerCacheOptions&) const;
            bool operator != (const PlayerCacheOptions&) const;
        };

        //! Timeline player cache information.
        struct PlayerCacheInfo
        {
            //! Video cache percentage used.
            float videoPercentage = 0.F;

            //! Cached video frames.
            std::vector<otime::TimeRange> videoFrames;

            //! Cached audio frames.
            std::vector<otime::TimeRange> audioFrames;

            bool operator == (const PlayerCacheInfo&) const;
            bool operator != (const PlayerCacheInfo&) const;
        };

        //! External time mode.
        enum class ExternalTimeMode
        {
            Relative,
            Absolute,

            Count,
            First = Relative
        };
        TLRENDER_ENUM(ExternalTimeMode);
        TLRENDER_ENUM_SERIALIZE(ExternalTimeMode);

        //! Get an external time from a source time.
        otime::RationalTime getExternalTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& externalTimeRange,
            ExternalTimeMode);

        //! Timeline player options.
        struct PlayerOptions
        {
            //! Cache options.
            PlayerCacheOptions cache;

            //! Timer mode.
            TimerMode timerMode = TimerMode::System;

            //! Audio buffer frame count.
            AudioBufferFrameCount audioBufferFrameCount = AudioBufferFrameCount::_256;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout = std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::milliseconds sleepTimeout = std::chrono::milliseconds(5);

            //! Current time.
            otime::RationalTime currentTime = time::invalidTime;

            //! External time mode.
            ExternalTimeMode externalTimeMode = ExternalTimeMode::Relative;

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
                const std::shared_ptr<system::Context>&,
                const PlayerOptions&);

            TimelinePlayer();

        public:
            ~TimelinePlayer();

            //! Create a new timeline player.
            static std::shared_ptr<TimelinePlayer> create(
                const std::shared_ptr<Timeline>&,
                const std::shared_ptr<system::Context>&,
                const PlayerOptions& = PlayerOptions());

            //! Get the context.
            const std::weak_ptr<system::Context>& getContext() const;

            //! Get the timeline.
            const std::shared_ptr<Timeline>& getTimeline() const;

            //! Get the path.
            const file::Path& getPath() const;

            //! Get the audio path.
            const file::Path& getAudioPath() const;

            //! Get the timeline player options.
            const PlayerOptions& getPlayerOptions() const;

            //! Get the timeline options.
            const Options& getOptions() const;

            //! \name Information
            ///@{

            //! Get the time range.
            const otime::TimeRange& getTimeRange() const;

            //! Get the I/O information. This information is retreived from
            //! the first clip in the timeline.
            const io::Info& getIOInfo() const;

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

            //! Use the time from a separate timeline player.
            void setExternalTime(const std::shared_ptr<TimelinePlayer>&);

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

            //! Observe the current video layer.
            std::shared_ptr<observer::IValue<uint16_t> > observeVideoLayer() const;

            //! Set the current video layer.
            void setVideoLayer(uint16_t);

            //! Observe the current video data.
            std::shared_ptr<observer::IValue<VideoData> > observeCurrentVideo() const;

            ///@}

            //! \name Audio
            ///@{

            //! Observe the audio volume.
            std::shared_ptr<observer::IValue<float> > observeVolume() const;

            //! Set the audio volume.
            void setVolume(float);

            //! Observe the audio mute.
            std::shared_ptr<observer::IValue<bool> > observeMute() const;

            //! Set the audio mute.
            void setMute(bool);

            //! Observe the audio sync offset (in seconds).
            std::shared_ptr<observer::IValue<double> > observeAudioOffset() const;

            //! Set the audio sync offset (in seconds).
            void setAudioOffset(double);

            //! Observe the current audio data.
            std::shared_ptr<observer::IList<AudioData> > observeCurrentAudio() const;

            ///@}

            //! \name Cache
            ///@{

            //! Observe the cache options.
            std::shared_ptr<observer::IValue<PlayerCacheOptions> > observeCacheOptions() const;

            //! Set the cache options.
            void setCacheOptions(const PlayerCacheOptions&);

            //! Observe the cache information.
            std::shared_ptr<observer::IValue<PlayerCacheInfo> > observeCacheInfo() const;

            ///@}

            //! Tick the timeline.
            void tick();

        private:
            TLRENDER_PRIVATE();
        };
    }
}

#include <tlTimeline/TimelinePlayerInline.h>
