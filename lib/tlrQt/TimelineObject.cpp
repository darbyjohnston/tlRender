// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineObject.h>

namespace tlr
{
    namespace qt
    {
        TimelineObject::TimelineObject(const std::string& fileName, QObject* parent) :
            QObject(parent)
        {
            _timeline = timeline::Timeline::create(fileName);

            _currentTimeObserver = Observer::Value<otime::RationalTime>::create(
                _timeline->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                {
                    Q_EMIT currentTimeChanged(value);
                });

            _playbackObserver = Observer::Value<timeline::Playback>::create(
                _timeline->observePlayback(),
                [this](timeline::Playback value)
                {
                    Q_EMIT playbackChanged(value);
                });

            _loopObserver = Observer::Value<timeline::Loop>::create(
                _timeline->observeLoop(),
                [this](timeline::Loop value)
                {
                    Q_EMIT loopChanged(value);
                });

            _currentImageObserver = Observer::Value<std::shared_ptr<imaging::Image> >::create(
                _timeline->observeCurrentImage(),
                [this](const std::shared_ptr<imaging::Image>& value)
                {
                    Q_EMIT currentImageChanged(value);
                });

            startTimer(0, Qt::PreciseTimer);
        }

        const otime::RationalTime& TimelineObject::getDuration() const
        {
            return _timeline->getDuration();
        }

        const imaging::Info& TimelineObject::getImageInfo() const
        {
            return _timeline->getImageInfo();
        }

        const otime::RationalTime& TimelineObject::getCurrentTime() const
        {
            return _timeline->observeCurrentTime()->get();
        }

        timeline::Playback TimelineObject::getPlayback() const
        {
            return _timeline->observePlayback()->get();
        }

        timeline::Loop TimelineObject::getLoop() const
        {
            return _timeline->observeLoop()->get();
        }

        const std::shared_ptr<tlr::imaging::Image>& TimelineObject::getCurrentImage() const
        {
            return _timeline->observeCurrentImage()->get();
        }

        void TimelineObject::setVideoQueueSize(size_t value)
        {
            _timeline->setVideoQueueSize(value);
        }

        void TimelineObject::setPlayback(tlr::timeline::Playback value)
        {
            _timeline->setPlayback(value);
        }

        void TimelineObject::stop()
        {
            _timeline->setPlayback(timeline::Playback::Stop);
        }

        void TimelineObject::forward()
        {
            _timeline->setPlayback(timeline::Playback::Forward);
        }

        void TimelineObject::togglePlayback()
        {
            _timeline->setPlayback(
                timeline::Playback::Stop == _timeline->observePlayback()->get() ?
                timeline::Playback::Forward :
                timeline::Playback::Stop);
        }

        void TimelineObject::startFrame()
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            _timeline->seek(otime::RationalTime(0, duration.rate()));
        }

        void TimelineObject::endFrame()
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            _timeline->seek(otime::RationalTime(duration.value() - 1, duration.rate()));
        }

        void TimelineObject::prevFrame()
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            const auto currentTime = _timeline->observeCurrentTime()->get();
            _timeline->seek(otime::RationalTime(currentTime.value() - 1, duration.rate()));
        }

        void TimelineObject::nextFrame()
        {
            _timeline->setPlayback(timeline::Playback::Stop);
            const auto duration = _timeline->getDuration();
            const auto currentTime = _timeline->observeCurrentTime()->get();
            _timeline->seek(otime::RationalTime(currentTime.value() + 1, duration.rate()));
        }

        void TimelineObject::setLoop(tlr::timeline::Loop value)
        {
            _timeline->setLoop(value);
        }

        void TimelineObject::seek(const otime::RationalTime& value)
        {
            _timeline->seek(value);
        }

        void TimelineObject::timerEvent(QTimerEvent*)
        {
            _timeline->tick();
        }
    }
}

