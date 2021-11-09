// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/Util.h>

#include <tlrCore/TimelinePlayer.h>

#include <QObject>

namespace tlr
{
    namespace qt
    {
        //! The timeline player timer interval.
        const int playerTimerInterval = 0;

        //! Timeline player.
        class TimelinePlayer : public QObject
        {
            Q_OBJECT
            Q_PROPERTY(tlr::timeline::VideoData video READ video NOTIFY videoChanged)

        public:
            TimelinePlayer(
                const file::Path&,
                const std::shared_ptr<core::Context>&,
                const timeline::PlayerOptions & = timeline::PlayerOptions(),
                const timeline::Options & = timeline::Options(),
                QObject* parent = nullptr);

            ~TimelinePlayer() override;
            
            //! Get the context.
            const std::weak_ptr<core::Context>& context() const;

            //! Get the timeline.
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline() const;

            //! Get the path.
            const file::Path& path() const;

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

            //! Get the A/V information. This information is retreived from
            //! the first clip in the timeline.
            const avio::Info& avInfo() const;

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

            //! Get the cache read ahead.
            int cacheReadAhead() const;

            //! Get the cache read behind.
            int cacheReadBehind() const;

            //! Get the cached video frames.
            const std::vector<otime::TimeRange>& cachedVideoFrames() const;

            ///@}

            //! \name Audio
            ///@{

            //! Get the audio volume.
            float volume() const;

            //! Get the audio mute.
            bool isMuted() const;

            //! Get the cached audio frames.
            const std::vector<otime::TimeRange>& cachedAudioFrames() const;

            ///@}

        public Q_SLOTS:
            //! \name Playback
            ///@{

            //! Set the playback speed.
            void setSpeed(double);

            //! Set the playback mode.
            void setPlayback(tlr::timeline::Playback);

            //! Stop playback.
            void stop();

            //! Forward playback.
            void forward();

            //! Reverse playback.
            void reverse();

            //! Toggle playback.
            void togglePlayback();

            //! Set the playback loop mode.
            void setLoop(tlr::timeline::Loop);

            ///@}

            //! \name Time
            ///@{

            //! Seek to the given time.
            void seek(const otime::RationalTime&);

            //! Time action.
            void timeAction(tlr::timeline::TimeAction);

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

            //! Set the cache read ahead.
            void setCacheReadAhead(int);

            //! Set the cache read behind.
            void setCacheReadBehind(int);

            ///@}

            //! \name Audio
            ///@{

            //! Set the audio volume.
            void setVolume(float);

            //! Set the audio mute.
            void setMute(bool);

            ///@}

        Q_SIGNALS:
            //! \name Playback
            ///@{

            //! This signal is emitted when the playback speed is changed.
            void speedChanged(double);

            //! This signal is emitted when the playback mode is changed.
            void playbackChanged(tlr::timeline::Playback);

            //! This signal is emitted when the playback loop mode is changed.
            void loopChanged(tlr::timeline::Loop);

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
            void videoChanged(const tlr::timeline::VideoData&);

            //! This signal is emitted when the cached video frames are changed.
            void cachedVideoFramesChanged(const std::vector<otime::TimeRange>&);

            ///@}

            //! \name Audio
            ///@{

            //! This signal is emitted when the audio volume is changed.
            void volumeChanged(float);

            //! This signal is emitted when the audio mute is changed.
            void muteChanged(bool);

            //! This signal is emitted when the cached audio frames are changed.
            void cachedAudioFramesChanged(const std::vector<otime::TimeRange>&);

            ///@}

        protected:
            void timerEvent(QTimerEvent*) override;

        private:
            TLR_PRIVATE();
        };
    }
}
