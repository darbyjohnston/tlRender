// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelinePlayer.h>

#include <tlrCore/Math.h>

namespace tlr
{
    namespace qt
    {
        struct TimelinePlayer::Private
        {
            std::shared_ptr<timeline::TimelinePlayer> timelinePlayer;

            std::shared_ptr<observer::ValueObserver<float> > speedObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> > inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<uint16_t> > videoLayerObserver;
            std::shared_ptr<observer::ValueObserver<timeline::VideoData> > videoObserver;
            std::shared_ptr<observer::ListObserver<otime::TimeRange> > cachedVideoFramesObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ListObserver<otime::TimeRange> > cachedAudioFramesObserver;
        };

        TimelinePlayer::TimelinePlayer(
            const file::Path& path,
            const std::shared_ptr<core::Context>& context,
            const timeline::Options& options,
            QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            p.timelinePlayer = timeline::TimelinePlayer::create(path, context, options);

            p.speedObserver = observer::ValueObserver<float>::create(
                p.timelinePlayer->observeSpeed(),
                [this](float value)
                {
                    Q_EMIT speedChanged(value);
                });

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

            p.videoLayerObserver = observer::ValueObserver<uint16_t>::create(
                p.timelinePlayer->observeVideoLayer(),
                [this](uint16_t value)
                {
                    Q_EMIT videoLayerChanged(value);
                });

            p.videoObserver = observer::ValueObserver<timeline::VideoData>::create(
                p.timelinePlayer->observeVideo(),
                [this](const timeline::VideoData& value)
                {
                    Q_EMIT videoChanged(value);
                });

            p.cachedVideoFramesObserver = observer::ListObserver<otime::TimeRange>::create(
                p.timelinePlayer->observeCachedVideoFrames(),
                [this](const std::vector<otime::TimeRange>& value)
                {
                    Q_EMIT cachedVideoFramesChanged(value);
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                p.timelinePlayer->observeVolume(),
                [this](float value)
                {
                    Q_EMIT volumeChanged(value);
                });

            p.muteObserver = observer::ValueObserver<bool>::create(
                p.timelinePlayer->observeMute(),
                [this](bool value)
                {
                    Q_EMIT muteChanged(value);
                });

            p.cachedAudioFramesObserver = observer::ListObserver<otime::TimeRange>::create(
                p.timelinePlayer->observeCachedAudioFrames(),
                [this](const std::vector<otime::TimeRange>& value)
                {
                    Q_EMIT cachedAudioFramesChanged(value);
                });

            startTimer(playerTimerInterval, Qt::PreciseTimer);
        }

        TimelinePlayer::~TimelinePlayer()
        {}
        
        const std::weak_ptr<core::Context>& TimelinePlayer::context() const
        {
            return _p->timelinePlayer->getContext();
        }

        const otio::SerializableObject::Retainer<otio::Timeline>& TimelinePlayer::timeline() const
        {
            return _p->timelinePlayer->getTimeline();
        }

        const file::Path& TimelinePlayer::path() const
        {
            return _p->timelinePlayer->getPath();
        }

        const otime::RationalTime& TimelinePlayer::globalStartTime() const
        {
            return _p->timelinePlayer->getGlobalStartTime();
        }

        const otime::RationalTime& TimelinePlayer::duration() const
        {
            return _p->timelinePlayer->getDuration();
        }

        const avio::Info& TimelinePlayer::avInfo() const
        {
            return _p->timelinePlayer->getAVInfo();
        }

       float TimelinePlayer::defaultSpeed() const
        {
            return _p->timelinePlayer->getDefaultSpeed();
        }

        float TimelinePlayer::speed() const
        {
            return _p->timelinePlayer->observeSpeed()->get();
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

        int TimelinePlayer::videoLayer() const
        {
            return _p->timelinePlayer->observeVideoLayer()->get();
        }

        const timeline::VideoData& TimelinePlayer::video() const
        {
            return _p->timelinePlayer->observeVideo()->get();
        }

        int TimelinePlayer::cacheReadAhead() const
        {
            return _p->timelinePlayer->getCacheReadAhead();
        }

        int TimelinePlayer::cacheReadBehind() const
        {
            return _p->timelinePlayer->getCacheReadBehind();
        }

        const std::vector<otime::TimeRange>& TimelinePlayer::cachedVideoFrames() const
        {
            return _p->timelinePlayer->observeCachedVideoFrames()->get();
        }

        float TimelinePlayer::volume() const
        {
            return _p->timelinePlayer->observeVolume()->get();
        }

        bool TimelinePlayer::isMuted() const
        {
            return _p->timelinePlayer->observeMute()->get();
        }

        const std::vector<otime::TimeRange>& TimelinePlayer::cachedAudioFrames() const
        {
            return _p->timelinePlayer->observeCachedAudioFrames()->get();
        }

        void TimelinePlayer::setSpeed(float value)
        {
            _p->timelinePlayer->setSpeed(value);
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

        void TimelinePlayer::setVideoLayer(int value)
        {
            _p->timelinePlayer->setVideoLayer(math::clamp(value, 0, static_cast<int>(std::numeric_limits<uint16_t>::max())));
        }

        void TimelinePlayer::setCacheReadAhead(int value)
        {
            _p->timelinePlayer->setCacheReadAhead(std::max(0, value));
        }

        void TimelinePlayer::setCacheReadBehind(int value)
        {
            _p->timelinePlayer->setCacheReadBehind(std::max(0, value));
        }

        void TimelinePlayer::setVolume(float value)
        {
            _p->timelinePlayer->setVolume(value);
        }

        void TimelinePlayer::setMute(bool value)
        {
            _p->timelinePlayer->setMute(value);
        }

        void TimelinePlayer::timerEvent(QTimerEvent*)
        {
            _p->timelinePlayer->tick();
        }
    }
}

