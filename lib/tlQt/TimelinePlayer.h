// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/Util.h>

#include <tlTimeline/TimelinePlayer.h>

#include <QObject>

namespace tl
{
    namespace qt
    {
        //! The timeline player sleep timeout.
        const std::chrono::milliseconds playerSleepTimeout(5);

        //! Qt based timeline player.
        class TimelinePlayer : public QObject
        {
            Q_OBJECT
            Q_PROPERTY(
                double defaultSpeed
                READ defaultSpeed)
            Q_PROPERTY(
                double speed
                READ speed
                WRITE setSpeed
                NOTIFY speedChanged)
            Q_PROPERTY(
                tl::timeline::Playback playback
                READ playback
                WRITE setPlayback
                NOTIFY playbackChanged)
            Q_PROPERTY(
                tl::timeline::Loop loop
                READ loop
                WRITE setLoop
                NOTIFY loopChanged)
            Q_PROPERTY(
                otime::RationalTime currentTime
                READ currentTime
                NOTIFY currentTimeChanged)
            Q_PROPERTY(
                otime::TimeRange inOutRange
                READ inOutRange
                WRITE setInOutRange
                NOTIFY inOutRangeChanged)
            Q_PROPERTY(
                int videoLayer
                READ videoLayer
                WRITE setVideoLayer
                NOTIFY videoLayerChanged)
            Q_PROPERTY(
                tl::timeline::VideoData
                video
                READ video
                NOTIFY videoChanged)
            Q_PROPERTY(
                float volume
                READ volume
                WRITE setVolume
                NOTIFY volumeChanged)
            Q_PROPERTY(
                bool mute
                READ isMuted
                WRITE setMute
                NOTIFY muteChanged)
            Q_PROPERTY(
                double audioOffset
                READ audioOffset
                WRITE setAudioOffset
                NOTIFY audioOffsetChanged)
            Q_PROPERTY(
                otime::RationalTime cacheReadAhead
                READ cacheReadAhead
                WRITE setCacheReadAhead
                NOTIFY cacheReadAheadChanged)
            Q_PROPERTY(
                otime::RationalTime cacheReadBehind
                READ cacheReadBehind
                WRITE setCacheReadBehind
                NOTIFY cacheReadBehindChanged)
            Q_PROPERTY(
                float cachePercentage
                READ cachePercentage
                NOTIFY cachePercentageChanged)

            void _init(
                const std::shared_ptr<timeline::TimelinePlayer>&,
                const std::shared_ptr<system::Context>&);

        public:
            TimelinePlayer(
                const std::shared_ptr<timeline::TimelinePlayer>&,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            ~TimelinePlayer() override;
            
            //! Get the context.
            const std::weak_ptr<system::Context>& context() const;

            //! Get the timeline player.
            const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer() const;

            //! Get the timeline.
            const std::shared_ptr<timeline::Timeline>& timeline() const;

            //! Get the path.
            const file::Path& path() const;

            //! Get the audio path.
            const file::Path& audioPath() const;

            //! Get the timeline player options.
            const timeline::PlayerOptions& getPlayerOptions() const;

            //! Get the timeline options.
            const timeline::Options& getOptions() const;

            //! \name Information
            ///@{

            //! Get the duration.
            const otime::RationalTime& duration() const;

            //! Get the global start time.
            const otime::RationalTime& globalStartTime() const;

            //! Get the I/O information. This information is retreived from
            //! the first clip in the timeline.
            const io::Info& ioInfo() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the default playback speed.
            double defaultSpeed() const;

            //! Get the playback speed.
            double speed() const;

            //! Get the playback mode.
            timeline::Playback playback() const;

            //! Get the playback loop mode.
            timeline::Loop loop() const;

            ///@}

            //! \name Time
            ///@{

            //! Get the current time.
            const otime::RationalTime& currentTime() const;

            ///@}

            //! \name In/Out Points
            ///@{

            //! Get the in/out points range.
            const otime::TimeRange& inOutRange() const;

            ///@}

            //! \name Video
            ///@{

            //! Get the current video layer.
            int videoLayer() const;

            //! Get the video.
            const timeline::VideoData& video() const;

            ///@}

            //! \name Audio
            ///@{

            //! Get the audio volume.
            float volume() const;

            //! Get the audio mute.
            bool isMuted() const;

            //! Get the audio sync offset (in seconds).
            double audioOffset() const;

            ///@}

            //! \name Cache
            ///@{

            //! Get the cache read ahead.
            otime::RationalTime cacheReadAhead() const;

            //! Get the cache read behind.
            otime::RationalTime cacheReadBehind() const;

            //! Get the cache percentage.
            float cachePercentage() const;

            //! Get the cached video frames.
            const std::vector<otime::TimeRange>& cachedVideoFrames() const;

            //! Get the cached audio frames.
            const std::vector<otime::TimeRange>& cachedAudioFrames() const;

            ///@}

        public Q_SLOTS:
            //! \name Playback
            ///@{

            //! Set the playback speed.
            void setSpeed(double);

            //! Set the playback mode.
            void setPlayback(tl::timeline::Playback);

            //! Stop playback.
            void stop();

            //! Forward playback.
            void forward();

            //! Reverse playback.
            void reverse();

            //! Toggle playback.
            void togglePlayback();

            //! Set the playback loop mode.
            void setLoop(tl::timeline::Loop);

            ///@}

            //! \name Time
            ///@{

            //! Seek to the given time.
            void seek(const otime::RationalTime&);

            //! Time action.
            void timeAction(tl::timeline::TimeAction);

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

            //! Set the current video layer.
            void setVideoLayer(int);

            ///@}

            //! \name Audio
            ///@{

            //! Set the audio volume.
            void setVolume(float);

            //! Increase the audio volume.
            void increaseVolume();

            //! Decrease the audio volume.
            void decreaseVolume();
                
            //! Set the audio mute.
            void setMute(bool);

            //! Set the audio sync offset (in seconds).
            void setAudioOffset(double);

            ///@}

            //! \name Cache
            ///@{

            //! Set the cache read ahead.
            void setCacheReadAhead(const otime::RationalTime&);

            //! Set the cache read behind.
            void setCacheReadBehind(const otime::RationalTime&);

            ///@}

        Q_SIGNALS:
            //! \name Playback
            ///@{

            //! This signal is emitted when the playback speed is changed.
            void speedChanged(double);

            //! This signal is emitted when the playback mode is changed.
            void playbackChanged(tl::timeline::Playback);

            //! This signal is emitted when the playback loop mode is changed.
            void loopChanged(tl::timeline::Loop);

            //! This signal is emitted when the current time is changed.
            void currentTimeChanged(const otime::RationalTime&);

            //! This signal is emitted when the in/out points range is changed.
            void inOutRangeChanged(const otime::TimeRange&);

            ///@}

            //! \name Video
            ///@{

            //! This signal is emitted when the current video layer is changed.
            void videoLayerChanged(int);

            //! This signal is emitted when the video is changed.
            void videoChanged(const tl::timeline::VideoData&);

            ///@}

            //! \name Audio
            ///@{

            //! This signal is emitted when the audio volume is changed.
            void volumeChanged(float);

            //! This signal is emitted when the audio mute is changed.
            void muteChanged(bool);

            //! This signal is emitted when the audio sync offset is changed.
            void audioOffsetChanged(double);

            ///@}

            //! \name Cache
            ///@{

            //! This signal is emitted when the cache read ahead has changed.
            void cacheReadAheadChanged(const otime::RationalTime&);

            //! This signal is emitted when the cache read behind has changed.
            void cacheReadBehindChanged(const otime::RationalTime&);

            //! This signal is emitted when the cache percentage has changed.
            void cachePercentageChanged(float);

            //! This signal is emitted when the cached video frames are changed.
            void cachedVideoFramesChanged(const std::vector<otime::TimeRange>&);

            //! This signal is emitted when the cached audio frames are changed.
            void cachedAudioFramesChanged(const std::vector<otime::TimeRange>&);

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    }
}
