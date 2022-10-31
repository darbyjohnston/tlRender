// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/TimelinePlayer.h>

#include <tlCore/Math.h>
#include <tlCore/Time.h>

#include <atomic>
#include <thread>

namespace tl
{
    namespace qt
    {
        struct TimelinePlayer::Private
        {
            std::shared_ptr<timeline::TimelinePlayer> timelinePlayer;

            std::shared_ptr<observer::ValueObserver<double> > speedObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> > inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<uint16_t> > videoLayerObserver;
            std::shared_ptr<observer::ValueObserver<timeline::VideoData> > currentVideoObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<double> > audioOffsetObserver;
            std::shared_ptr<observer::ListObserver<timeline::AudioData> > currentAudioObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheOptions> > cacheOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;
        };

        void TimelinePlayer::_init(
            const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.timelinePlayer = timelinePlayer;

            p.speedObserver = observer::ValueObserver<double>::create(
                p.timelinePlayer->observeSpeed(),
                [this](double value)
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

            p.currentVideoObserver = observer::ValueObserver<timeline::VideoData>::create(
                p.timelinePlayer->observeCurrentVideo(),
                [this](const timeline::VideoData& value)
                {
                    Q_EMIT currentVideoChanged(value);
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

            p.audioOffsetObserver = observer::ValueObserver<double>::create(
                p.timelinePlayer->observeAudioOffset(),
                [this](double value)
                {
                    Q_EMIT audioOffsetChanged(value);
                });

            p.currentAudioObserver = observer::ListObserver<timeline::AudioData>::create(
                p.timelinePlayer->observeCurrentAudio(),
                [this](const std::vector<timeline::AudioData>& value)
                {
                    Q_EMIT currentAudioChanged(value);
                });

            p.cacheOptionsObserver = observer::ValueObserver<timeline::PlayerCacheOptions>::create(
                p.timelinePlayer->observeCacheOptions(),
                [this](const timeline::PlayerCacheOptions& value)
                {
                    Q_EMIT cacheOptionsChanged(value);
                });

            p.cacheInfoObserver = observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                p.timelinePlayer->observeCacheInfo(),
                [this](const timeline::PlayerCacheInfo& value)
                {
                    Q_EMIT cacheInfoChanged(value);
                });

            startTimer(5, Qt::PreciseTimer);
        }

        TimelinePlayer::TimelinePlayer(
            const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            _init(timelinePlayer, context);
        }

        TimelinePlayer::~TimelinePlayer()
        {}
        
        const std::weak_ptr<system::Context>& TimelinePlayer::context() const
        {
            return _p->timelinePlayer->getContext();
        }

        const std::shared_ptr<timeline::TimelinePlayer>& TimelinePlayer::timelinePlayer() const
        {
            return _p->timelinePlayer;
        }

        const std::shared_ptr<timeline::Timeline>& TimelinePlayer::timeline() const
        {
            return _p->timelinePlayer->getTimeline();
        }

        const file::Path& TimelinePlayer::path() const
        {
            return _p->timelinePlayer->getPath();
        }

        const file::Path& TimelinePlayer::audioPath() const
        {
            return _p->timelinePlayer->getAudioPath();
        }

        const timeline::PlayerOptions& TimelinePlayer::getPlayerOptions() const
        {
            return _p->timelinePlayer->getPlayerOptions();
        }

        const timeline::Options& TimelinePlayer::getOptions() const
        {
            return _p->timelinePlayer->getOptions();
        }

        const otime::TimeRange& TimelinePlayer::timeRange() const
        {
            return _p->timelinePlayer->getTimeRange();
        }

        const io::Info& TimelinePlayer::ioInfo() const
        {
            return _p->timelinePlayer->getIOInfo();
        }

        double TimelinePlayer::defaultSpeed() const
        {
            return _p->timelinePlayer->getDefaultSpeed();
        }

        double TimelinePlayer::speed() const
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

        const timeline::VideoData& TimelinePlayer::currentVideo() const
        {
            return _p->timelinePlayer->observeCurrentVideo()->get();
        }

        float TimelinePlayer::volume() const
        {
            return _p->timelinePlayer->observeVolume()->get();
        }

        bool TimelinePlayer::isMuted() const
        {
            return _p->timelinePlayer->observeMute()->get();
        }

        double TimelinePlayer::audioOffset() const
        {
            return _p->timelinePlayer->observeAudioOffset()->get();
        }

        const std::vector<timeline::AudioData>& TimelinePlayer::currentAudio() const
        {
            return _p->timelinePlayer->observeCurrentAudio()->get();
        }

        const timeline::PlayerCacheOptions& TimelinePlayer::cacheOptions() const
        {
            return _p->timelinePlayer->observeCacheOptions()->get();
        }

        const timeline::PlayerCacheInfo& TimelinePlayer::cacheInfo() const
        {
            return _p->timelinePlayer->observeCacheInfo()->get();
        }

        void TimelinePlayer::setSpeed(double value)
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

        void TimelinePlayer::setVolume(float value)
        {
            _p->timelinePlayer->setVolume(value);
        }

        void TimelinePlayer::increaseVolume()
        {
            _p->timelinePlayer->increaseVolume();
        }

        void TimelinePlayer::decreaseVolume()
        {
            _p->timelinePlayer->decreaseVolume();
        }

        void TimelinePlayer::setMute(bool value)
        {
            _p->timelinePlayer->setMute(value);
        }

        void TimelinePlayer::setAudioOffset(double value)
        {
            _p->timelinePlayer->setAudioOffset(value);
        }

        void TimelinePlayer::setCacheOptions(const timeline::PlayerCacheOptions& value)
        {
            _p->timelinePlayer->setCacheOptions(value);
        }

        void TimelinePlayer::timerEvent(QTimerEvent* event)
        {
            _p->timelinePlayer->tick();
        }
    }
}

