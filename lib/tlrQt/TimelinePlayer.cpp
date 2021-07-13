// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelinePlayer.h>

namespace tlr
{
    namespace qt
    {
        struct TimelinePlayer::Private
        {
            std::shared_ptr<timeline::TimelinePlayer> timelinePlayer;

            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> > inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Frame> > frameObserver;
            std::shared_ptr<observer::ListObserver<otime::TimeRange> > cachedFramesObserver;
        };

        TimelinePlayer::TimelinePlayer(const QString& fileName, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            p.timelinePlayer = timeline::TimelinePlayer::create(fileName.toLatin1().data());

            p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                p.timelinePlayer->observePlayback(),
                [this](timeline::Playback value)
                {
                    Q_EMIT playbackChanged(value);
                });

            p.loopObserver = observer::ValueObserver<timeline::Loop>::create(
                p.timelinePlayer->observeLoop(),
                [this](timeline::Loop value)
                {
                    Q_EMIT loopChanged(value);
                });

            p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                p.timelinePlayer->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                {
                    Q_EMIT currentTimeChanged(value);
                });

            p.inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                p.timelinePlayer->observeInOutRange(),
                [this](const otime::TimeRange value)
                {
                    Q_EMIT inOutRangeChanged(value);
                });

            p.frameObserver = observer::ValueObserver<timeline::Frame>::create(
                p.timelinePlayer->observeFrame(),
                [this](const timeline::Frame& value)
                {
                    Q_EMIT frameChanged(value);
                });

            p.cachedFramesObserver = observer::ListObserver<otime::TimeRange>::create(
                p.timelinePlayer->observeCachedFrames(),
                [this](const std::vector<otime::TimeRange>& value)
                {
                    Q_EMIT cachedFramesChanged(value);
                });

            startTimer(playerTimerInterval, Qt::PreciseTimer);
        }

        QString TimelinePlayer::fileName() const
        {
            return _p->timelinePlayer->getFileName().c_str();
        }

        const otime::RationalTime& TimelinePlayer::globalStartTime() const
        {
            return _p->timelinePlayer->getGlobalStartTime();
        }

        const otime::RationalTime& TimelinePlayer::duration() const
        {
            return _p->timelinePlayer->getDuration();
        }

        const imaging::Info& TimelinePlayer::imageInfo() const
        {
            return _p->timelinePlayer->getImageInfo();
        }

        timeline::Playback TimelinePlayer::playback() const
        {
            return _p->timelinePlayer->observePlayback()->get();
        }

        timeline::Loop TimelinePlayer::loop() const
        {
            return _p->timelinePlayer->observeLoop()->get();
        }

        const otime::RationalTime& TimelinePlayer::currentTime() const
        {
            return _p->timelinePlayer->observeCurrentTime()->get();
        }

        const otime::TimeRange& TimelinePlayer::inOutRange() const
        {
            return _p->timelinePlayer->observeInOutRange()->get();
        }

        const timeline::Frame& TimelinePlayer::frame() const
        {
            return _p->timelinePlayer->observeFrame()->get();
        }

        const std::vector<otime::TimeRange>& TimelinePlayer::cachedFrames() const
        {
            return _p->timelinePlayer->observeCachedFrames()->get();
        }

        void TimelinePlayer::setPlayback(timeline::Playback value)
        {
            _p->timelinePlayer->setPlayback(value);
        }

        void TimelinePlayer::stop()
        {
            _p->timelinePlayer->setPlayback(timeline::Playback::Stop);
        }

        void TimelinePlayer::forward()
        {
            _p->timelinePlayer->setPlayback(timeline::Playback::Forward);
        }

        void TimelinePlayer::reverse()
        {
            _p->timelinePlayer->setPlayback(timeline::Playback::Reverse);
        }

        void TimelinePlayer::togglePlayback()
        {
            _p->timelinePlayer->setPlayback(
                timeline::Playback::Stop == _p->timelinePlayer->observePlayback()->get() ?
                timeline::Playback::Forward :
                timeline::Playback::Stop);
        }

        void TimelinePlayer::setLoop(timeline::Loop value)
        {
            _p->timelinePlayer->setLoop(value);
        }

        void TimelinePlayer::seek(const otime::RationalTime& value)
        {
            _p->timelinePlayer->seek(value);
        }

        void TimelinePlayer::timeAction(timeline::TimeAction value)
        {
            _p->timelinePlayer->timeAction(value);
        }

        void TimelinePlayer::start()
        {
            _p->timelinePlayer->start();
        }

        void TimelinePlayer::end()
        {
            _p->timelinePlayer->end();
        }

        void TimelinePlayer::framePrev()
        {
            _p->timelinePlayer->framePrev();
        }

        void TimelinePlayer::frameNext()
        {
            _p->timelinePlayer->frameNext();
        }

        void TimelinePlayer::setInOutRange(const otime::TimeRange& value)
        {
            _p->timelinePlayer->setInOutRange(value);
        }

        void TimelinePlayer::setInPoint()
        {
            _p->timelinePlayer->setInPoint();
        }

        void TimelinePlayer::resetInPoint()
        {
            _p->timelinePlayer->resetInPoint();
        }

        void TimelinePlayer::setOutPoint()
        {
            _p->timelinePlayer->setOutPoint();
        }

        void TimelinePlayer::resetOutPoint()
        {
            _p->timelinePlayer->resetOutPoint();
        }

        void TimelinePlayer::setFrameCacheReadAhead(int value)
        {
            _p->timelinePlayer->setFrameCacheReadAhead(value);
        }

        void TimelinePlayer::setFrameCacheReadBehind(int value)
        {
            _p->timelinePlayer->setFrameCacheReadBehind(value);
        }

        void TimelinePlayer::timerEvent(QTimerEvent*)
        {
            _p->timelinePlayer->tick();
        }
    }
}

