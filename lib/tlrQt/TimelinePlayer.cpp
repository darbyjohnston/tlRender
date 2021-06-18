// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelinePlayer.h>

namespace tlr
{
    namespace qt
    {
        TimelinePlayer::TimelinePlayer(const QString& fileName, QObject* parent) :
            QObject(parent)
        {
            _timelinePlayer = timeline::TimelinePlayer::create(fileName.toLatin1().data());

            _playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                _timelinePlayer->observePlayback(),
                [this](timeline::Playback value)
                {
                    Q_EMIT playbackChanged(value);
                });

            _loopObserver = observer::ValueObserver<timeline::Loop>::create(
                _timelinePlayer->observeLoop(),
                [this](timeline::Loop value)
                {
                    Q_EMIT loopChanged(value);
                });

            _currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                _timelinePlayer->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                {
                    Q_EMIT currentTimeChanged(value);
                });

            _inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                _timelinePlayer->observeInOutRange(),
                [this](const otime::TimeRange value)
                {
                    Q_EMIT inOutRangeChanged(value);
                });

            _frameObserver = observer::ValueObserver<timeline::RenderFrame>::create(
                _timelinePlayer->observeFrame(),
                [this](const timeline::RenderFrame& value)
                {
                    Q_EMIT frameChanged(value);
                });

            _cachedFramesObserver = observer::ListObserver<otime::TimeRange>::create(
                _timelinePlayer->observeCachedFrames(),
                [this](const std::vector<otime::TimeRange>& value)
                {
                    Q_EMIT cachedFramesChanged(value);
                });

            startTimer(playerTimerInterval, Qt::PreciseTimer);
        }

        QString TimelinePlayer::fileName() const
        {
            return _timelinePlayer->getFileName().c_str();
        }

        const otime::RationalTime& TimelinePlayer::globalStartTime() const
        {
            return _timelinePlayer->getGlobalStartTime();
        }

        const otime::RationalTime& TimelinePlayer::duration() const
        {
            return _timelinePlayer->getDuration();
        }

        const imaging::Info& TimelinePlayer::imageInfo() const
        {
            return _timelinePlayer->getImageInfo();
        }

        timeline::Playback TimelinePlayer::playback() const
        {
            return _timelinePlayer->observePlayback()->get();
        }

        timeline::Loop TimelinePlayer::loop() const
        {
            return _timelinePlayer->observeLoop()->get();
        }

        const otime::RationalTime& TimelinePlayer::currentTime() const
        {
            return _timelinePlayer->observeCurrentTime()->get();
        }

        const otime::TimeRange& TimelinePlayer::inOutRange() const
        {
            return _timelinePlayer->observeInOutRange()->get();
        }

        const timeline::RenderFrame& TimelinePlayer::frame() const
        {
            return _timelinePlayer->observeFrame()->get();
        }

        const std::vector<otime::TimeRange>& TimelinePlayer::cachedFrames() const
        {
            return _timelinePlayer->observeCachedFrames()->get();
        }

        void TimelinePlayer::setPlayback(timeline::Playback value)
        {
            _timelinePlayer->setPlayback(value);
        }

        void TimelinePlayer::stop()
        {
            _timelinePlayer->setPlayback(timeline::Playback::Stop);
        }

        void TimelinePlayer::forward()
        {
            _timelinePlayer->setPlayback(timeline::Playback::Forward);
        }

        void TimelinePlayer::reverse()
        {
            _timelinePlayer->setPlayback(timeline::Playback::Reverse);
        }

        void TimelinePlayer::togglePlayback()
        {
            _timelinePlayer->setPlayback(
                timeline::Playback::Stop == _timelinePlayer->observePlayback()->get() ?
                timeline::Playback::Forward :
                timeline::Playback::Stop);
        }

        void TimelinePlayer::setLoop(timeline::Loop value)
        {
            _timelinePlayer->setLoop(value);
        }

        void TimelinePlayer::seek(const otime::RationalTime& value)
        {
            _timelinePlayer->seek(value);
        }

        void TimelinePlayer::timeAction(timeline::TimeAction value)
        {
            _timelinePlayer->timeAction(value);
        }

        void TimelinePlayer::start()
        {
            _timelinePlayer->start();
        }

        void TimelinePlayer::end()
        {
            _timelinePlayer->end();
        }

        void TimelinePlayer::framePrev()
        {
            _timelinePlayer->framePrev();
        }

        void TimelinePlayer::frameNext()
        {
            _timelinePlayer->frameNext();
        }

        void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
        {
            _timelinePlayer->setInOutRange(value);
        }

        void TimelinePlayer::setInPoint()
        {
            _timelinePlayer->setInPoint();
        }

        void TimelinePlayer::resetInPoint()
        {
            _timelinePlayer->resetInPoint();
        }

        void TimelinePlayer::setOutPoint()
        {
            _timelinePlayer->setOutPoint();
        }

        void TimelinePlayer::resetOutPoint()
        {
            _timelinePlayer->resetOutPoint();
        }

        void TimelinePlayer::setFrameCacheReadAhead(int value)
        {
            _timelinePlayer->setFrameCacheReadAhead(value);
        }

        void TimelinePlayer::setFrameCacheReadBehind(int value)
        {
            _timelinePlayer->setFrameCacheReadBehind(value);
        }

        void TimelinePlayer::timerEvent(QTimerEvent*)
        {
            _timelinePlayer->tick();
        }
    }
}

