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
                tl::timeline::PlayerCacheOptions cacheOptions
                READ cacheOptions
                WRITE setCacheOptions
                NOTIFY cacheOptionsChanged)
            Q_PROPERTY(
                tl::timeline::PlayerCacheInfo cacheInfo
                READ cacheInfo
                NOTIFY cacheInfoChanged)

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

            //! Get the time range.
            const otime::TimeRange& timeRange() const;

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

            //! Get the cache options.
            const timeline::PlayerCacheOptions& cacheOptions() const;

            //! Get the cache information.
            const timeline::PlayerCacheInfo& cacheInfo() const;

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

            //! Set the cache options.
            void setCacheOptions(const tl::timeline::PlayerCacheOptions&);

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

            //! This signal is emitted when the cache options have changed.
            void cacheOptionsChanged(const tl::timeline::PlayerCacheOptions&);

            //! This signal is emitted when the cache information has changed.
            void cacheInfoChanged(const tl::timeline::PlayerCacheInfo&);

            ///@}

        protected:
            void timerEvent(QTimerEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
