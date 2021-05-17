// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Timeline.h>

#include <QObject>

namespace tlr
{
    namespace qt
    {
        class TimelineObject : public QObject
        {
            Q_OBJECT

        public:
            TimelineObject(const QString& fileName, QObject* parent = nullptr);
            ~TimelineObject() override;

            //! \name Information
            ///@{

            //! Get the file name.
            QString fileName() const;

            //! Get the duration.
            const otime::RationalTime& duration() const;

            //! Get the global start time.
            const otime::RationalTime& globalStartTime() const;

            //! Get the image info.
            const imaging::Info& imageInfo() const;

            //! Get the clip time ranges.
            std::vector<otime::TimeRange> clipRanges() const;

            ///@}

            //! \name Playback
            ///@{

            //! Get the playback mode.
            timeline::Playback playback() const;

            //! Get the playback loop mode.
            timeline::Loop loop() const;

            //! Get the current time.
            const otime::RationalTime& currentTime() const;

            //! Get the in/out points range.
            const otime::TimeRange& inOutRange() const;

            ///@}

            //! \name Frames
            ///@{

            //! Get the current frame.
            const io::VideoFrame& frame() const;

            //! Get the frame cache read ahead.
            int frameCacheReadAhead();

            //! Get the frame cache read behind.
            int frameCacheReadBehind();

            //! Get the cached frames.
            const std::vector<otime::TimeRange>& cachedFrames() const;

            ///@}

        public Q_SLOTS:
            //! \name Playback
            ///@{

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

            //! Go to the previous clip.
            void clipPrev();

            //! Go to the next clip.
            void clipNext();

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

            //! Set the frame cache read ahead.
            void setFrameCacheReadAhead(int);

            //! Set the frame cache read behind.
            void setFrameCacheReadBehind(int);

            ///@}

        Q_SIGNALS:
            //! \name Playback
            ///@{

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

            //! This signal is emitted when the current frame is changed.
            void frameChanged(const tlr::io::VideoFrame&);

            //! This signal is emitted when the cached frames are changed.
            void cachedFramesChanged(const std::vector<otime::TimeRange>&);

            ///@}

        protected:
            void timerEvent(QTimerEvent*) override;

        private:
            std::shared_ptr<timeline::Timeline> _timeline;

            std::shared_ptr<Observer::Value<timeline::Playback> > _playbackObserver;
            std::shared_ptr<Observer::Value<timeline::Loop> > _loopObserver;
            std::shared_ptr<Observer::Value<otime::RationalTime> > _currentTimeObserver;
            std::shared_ptr<Observer::Value<otime::TimeRange> > _inOutRangeObserver;
            std::shared_ptr<Observer::Value<io::VideoFrame> > _frameObserver;
            std::shared_ptr<Observer::List<otime::TimeRange> > _cachedFramesObserver;
        };
    }
}