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
            Q_PROPERTY(tlr::timeline::Frame frame READ frame NOTIFY frameChanged)

        public:
            TimelinePlayer(
                const file::Path&,
                const std::shared_ptr<core::Context>&,
                QObject* parent = nullptr);

            ~TimelinePlayer() override;
            
            //! Get the context.
            const std::weak_ptr<core::Context>& context() const;

            //! Get the timeline.
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline() const;

            //! Get the path.
            const file::Path& path() const;

            //! \name Information
            ///@{

            //! Get the duration.
            const otime::RationalTime& duration() const;

            //! Get the global start time.
            const otime::RationalTime& globalStartTime() const;

            //! Get the video information. This information is retreived from
            //! the first clip in the timeline. The vector represents the
            //! different layers that are available.
            const std::vector<imaging::Info>& videoInfo() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the default playback speed.
            float defaultSpeed() const;

            //! Get the playback speed.
            float speed() const;

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

            //! \name Frames
            ///@{

            //! Get the current video layer.
            int videoLayer() const;

            //! Get the current frame.
            const timeline::Frame& frame() const;

            //! Get the frame cache read ahead.
            int frameCacheReadAhead();

            //! Get the frame cache read behind.
            int frameCacheReadBehind();

            //! Get the cached frames.
            const std::vector<otime::TimeRange>& cachedFrames() const;

            ///@}

            //! \name Options
            ///@{

            //! Get the number of frame requests.
            int requestCount() const;

            //! Get the frame request timeout (milliseconds).
            int requestTimeout() const;

            ///@}

        public Q_SLOTS:
            //! \name Playback
            ///@{

            //! Set the playback speed.
            void setSpeed(float);

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

            //! \name Frames
            ///@{

            //! Set the current video layer.
            void setVideoLayer(int);

            //! Set the frame cache read ahead.
            void setFrameCacheReadAhead(int);

            //! Set the frame cache read behind.
            void setFrameCacheReadBehind(int);

            ///@}

            //! \name Options
            ///@{

            //! Set the number of frame requests.
            void setRequestCount(int);

            //! Set the frame request timeout (milliseconds).
            void setRequestTimeout(int);

            ///@}

        Q_SIGNALS:
            //! \name Playback
            ///@{

            //! This signal is emitted when the playback speed is changed.
            void speedChanged(float);

            //! This signal is emitted when the playback mode is changed.
            void playbackChanged(tlr::timeline::Playback);

            //! This signal is emitted when the playback loop mode is changed.
            void loopChanged(tlr::timeline::Loop);

            //! This signal is emitted when the current time is changed.
            void currentTimeChanged(const otime::RationalTime&);

            //! This signal is emitted when the in/out points range is changed.
            void inOutRangeChanged(const otime::TimeRange&);

            ///@}

            //! \name Frames
            ///@{

            //! This signal is emitted when the current video layer is changed.
            void videoLayerChanged(int);

            //! This signal is emitted when the current frame is changed.
            void frameChanged(const tlr::timeline::Frame&);

            //! This signal is emitted when the cached frames are changed.
            void cachedFramesChanged(const std::vector<otime::TimeRange>&);

            ///@}

        protected:
            void timerEvent(QTimerEvent*) override;

        private:
            TLR_PRIVATE();
        };
    }
}
