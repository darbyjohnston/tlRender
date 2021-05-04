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
            TimelineObject(const std::string& fileName, QObject* parent = nullptr);

            //! Get the duration.
            const otime::RationalTime& getDuration() const;

            //! Get the image info (from the first clip in the timeline).
            const imaging::Info& getImageInfo() const;

            //! Get the current time.
            const otime::RationalTime& getCurrentTime() const;

            //! Get the playback mode.
            timeline::Playback getPlayback() const;

            //! Get the playback loop mode.
            timeline::Loop getLoop() const;

            //! Get the current image.
            const std::shared_ptr<tlr::imaging::Image>& getCurrentImage() const;

            //! Set the I/O video queue size.
            void setVideoQueueSize(size_t);

        public Q_SLOTS:
            //! Set the playback mode.
            void setPlayback(tlr::timeline::Playback);

            //! Stop playback.
            void stop();

            //! Forward playback.
            void forward();

            //! Toggle playback.
            void togglePlayback();

            //! Go to the start frame.
            void startFrame();

            //! Go to the end frame.
            void endFrame();

            //! Go to the previous frame.
            void prevFrame();

            //! Go to the next frame.
            void nextFrame();

            //! Set the playback loop mode.
            void setLoop(tlr::timeline::Loop);

            //! Seek.
            void seek(const otime::RationalTime&);

        Q_SIGNALS:
            //! This signal is emitted when the current time is changed.
            void currentTimeChanged(const otime::RationalTime&);

            //! This signal is emitted when the playback mode is changed.
            void playbackChanged(tlr::timeline::Playback);

            //! This signal is emitted when the playback loop mode is changed.
            void loopChanged(tlr::timeline::Loop);

            //! This signal is emitted whent the current image is changed.
            void currentImageChanged(const std::shared_ptr<tlr::imaging::Image>&);

        protected:
            void timerEvent(QTimerEvent*) override;

        private:
            std::shared_ptr<timeline::Timeline> _timeline;

            std::shared_ptr<Observer::Value<otime::RationalTime> > _currentTimeObserver;
            std::shared_ptr<Observer::Value<timeline::Playback> > _playbackObserver;
            std::shared_ptr<Observer::Value<timeline::Loop> > _loopObserver;
            std::shared_ptr<Observer::Value<std::shared_ptr<imaging::Image> > > _currentImageObserver;
        };
    }
}