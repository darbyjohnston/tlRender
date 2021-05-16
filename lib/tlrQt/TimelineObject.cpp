// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineObject.h>

namespace tlr
{
    namespace qt
    {
        TimelineObject::TimelineObject(const QString& fileName, QObject* parent) :
            QObject(parent)
        {
            _timeline = timeline::Timeline::create(fileName.toLatin1().data());

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

            _currentTimeObserver = Observer::Value<otime::RationalTime>::create(
                _timeline->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                {
                    Q_EMIT currentTimeChanged(value);
                });

            _inOutRangeObserver = Observer::Value<otime::TimeRange>::create(
                _timeline->observeInOutRange(),
                [this](const otime::TimeRange value)
                {
                    Q_EMIT inOutRangeChanged(value);
                });

            _frameObserver = Observer::Value<io::VideoFrame>::create(
                _timeline->observeFrame(),
                [this](const io::VideoFrame& value)
                {
                    Q_EMIT frameChanged(value);
                });

            _cachedFramesObserver = Observer::List<otime::TimeRange>::create(
                _timeline->observeCachedFrames(),
                [this](const std::vector<otime::TimeRange>& value)
                {
                    Q_EMIT cachedFramesChanged(value);
                });

            startTimer(0, Qt::PreciseTimer);
        }

        TimelineObject::~TimelineObject()
        {}

        QString TimelineObject::fileName() const
        {
            return _timeline->getFileName().c_str();
        }

        const otime::RationalTime& TimelineObject::globalStartTime() const
        {
            return _timeline->getGlobalStartTime();
        }

        const otime::RationalTime& TimelineObject::duration() const
        {
            return _timeline->getDuration();
        }

        const imaging::Info& TimelineObject::imageInfo() const
        {
            return _timeline->getImageInfo();
        }

        timeline::Playback TimelineObject::playback() const
        {
            return _timeline->observePlayback()->get();
        }

        timeline::Loop TimelineObject::loop() const
        {
            return _timeline->observeLoop()->get();
        }

        const otime::RationalTime& TimelineObject::currentTime() const
        {
            return _timeline->observeCurrentTime()->get();
        }

        const otime::TimeRange& TimelineObject::inOutRange() const
        {
            return _timeline->observeInOutRange()->get();
        }

        const io::VideoFrame& TimelineObject::frame() const
        {
            return _timeline->observeFrame()->get();
        }

        const std::vector<otime::TimeRange>& TimelineObject::cachedFrames() const
        {
            return _timeline->observeCachedFrames()->get();
        }

        void TimelineObject::setPlayback(timeline::Playback value)
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

        void TimelineObject::reverse()
        {
            _timeline->setPlayback(timeline::Playback::Reverse);
        }

        void TimelineObject::togglePlayback()
        {
            _timeline->setPlayback(
                timeline::Playback::Stop == _timeline->observePlayback()->get() ?
                timeline::Playback::Forward :
                timeline::Playback::Stop);
        }

        void TimelineObject::setLoop(timeline::Loop value)
        {
            _timeline->setLoop(value);
        }

        void TimelineObject::seek(const otime::RationalTime& value)
        {
            _timeline->seek(value);
        }

        void TimelineObject::frame(timeline::Frame value)
        {
            _timeline->frame(value);
        }

        void TimelineObject::start()
        {
            _timeline->start();
        }

        void TimelineObject::end()
        {
            _timeline->end();
        }

        void TimelineObject::prev()
        {
            _timeline->prev();
        }

        void TimelineObject::next()
        {
            _timeline->next();
        }

        void TimelineObject::setInOutRange(const otime::TimeRange& value)
        {
            _timeline->setInOutRange(value);
        }

        void TimelineObject::setInPoint()
        {
            _timeline->setInPoint();
        }

        void TimelineObject::resetInPoint()
        {
            _timeline->resetInPoint();
        }

        void TimelineObject::setOutPoint()
        {
            _timeline->setOutPoint();
        }

        void TimelineObject::resetOutPoint()
        {
            _timeline->resetOutPoint();
        }

        void TimelineObject::setFrameCacheReadAhead(int value)
        {
            _timeline->setFrameCacheReadAhead(value);
        }

        void TimelineObject::setFrameCacheReadBehind(int value)
        {
            _timeline->setFrameCacheReadBehind(value);
        }

        void TimelineObject::timerEvent(QTimerEvent*)
        {
            _timeline->tick();
        }
    }
}

