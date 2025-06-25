// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQt/PlayerObject.h>

#include <tlCore/Time.h>

#include <feather-tk/core/Math.h>

#include <QTimer>

#include <atomic>
#include <thread>

namespace tl
{
    namespace qt
    {
        namespace
        {
            const size_t timeout = 5;
        }

        struct PlayerObject::Private
        {
            std::shared_ptr<timeline::Player> player;
            std::unique_ptr<QTimer> timer;

            std::shared_ptr<feather_tk::ValueObserver<double> > speedObserver;
            std::shared_ptr<feather_tk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<feather_tk::ValueObserver<timeline::Loop> > loopObserver;
            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::TimeRange> > inOutRangeObserver;
            std::shared_ptr<feather_tk::ListObserver<std::shared_ptr<timeline::Timeline> > > compareObserver;
            std::shared_ptr<feather_tk::ValueObserver<timeline::CompareTime> > compareTimeObserver;
            std::shared_ptr<feather_tk::ValueObserver<io::Options> > ioOptionsObserver;
            std::shared_ptr<feather_tk::ValueObserver<int> > videoLayerObserver;
            std::shared_ptr<feather_tk::ListObserver<int> > compareVideoLayersObserver;
            std::shared_ptr<feather_tk::ListObserver<timeline::VideoData> > currentVideoObserver;
            std::shared_ptr<feather_tk::ValueObserver<audio::DeviceID> > audioDeviceObserver;
            std::shared_ptr<feather_tk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<feather_tk::ValueObserver<bool> > muteObserver;
            std::shared_ptr<feather_tk::ListObserver<bool> > channelMuteObserver;
            std::shared_ptr<feather_tk::ValueObserver<double> > audioOffsetObserver;
            std::shared_ptr<feather_tk::ListObserver<timeline::AudioData> > currentAudioObserver;
            std::shared_ptr<feather_tk::ValueObserver<timeline::PlayerCacheOptions> > cacheOptionsObserver;
            std::shared_ptr<feather_tk::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;
        };

        void PlayerObject::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<timeline::Player>& player)
        {
            FEATHER_TK_P();

            p.player = player;

            p.speedObserver = feather_tk::ValueObserver<double>::create(
                p.player->observeSpeed(),
                [this](double value)
                {
                    Q_EMIT speedChanged(value);
                });

            p.playbackObserver = feather_tk::ValueObserver<timeline::Playback>::create(
                p.player->observePlayback(),
                [this](timeline::Playback value)
                {
                    Q_EMIT playbackChanged(value);
                });

            p.loopObserver = feather_tk::ValueObserver<timeline::Loop>::create(
                p.player->observeLoop(),
                [this](timeline::Loop value)
                {
                    Q_EMIT loopChanged(value);
                });

            p.currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                p.player->observeCurrentTime(),
                [this](const OTIO_NS::RationalTime& value)
                {
                    Q_EMIT currentTimeChanged(value);
                });

            p.inOutRangeObserver = feather_tk::ValueObserver<OTIO_NS::TimeRange>::create(
                p.player->observeInOutRange(),
                [this](const OTIO_NS::TimeRange value)
                {
                    Q_EMIT inOutRangeChanged(value);
                });

            p.compareObserver = feather_tk::ListObserver<std::shared_ptr<timeline::Timeline> >::create(
                p.player->observeCompare(),
                [this](const std::vector<std::shared_ptr<timeline::Timeline> >& value)
                {
                    Q_EMIT compareChanged(value);
                });

            p.compareTimeObserver = feather_tk::ValueObserver<timeline::CompareTime>::create(
                p.player->observeCompareTime(),
                [this](timeline::CompareTime value)
                {
                    Q_EMIT compareTimeChanged(value);
                });

            p.ioOptionsObserver = feather_tk::ValueObserver<io::Options>::create(
                p.player->observeIOOptions(),
                [this](const io::Options& value)
                {
                    Q_EMIT ioOptionsChanged(value);
                });

            p.videoLayerObserver = feather_tk::ValueObserver<int>::create(
                p.player->observeVideoLayer(),
                [this](int value)
                {
                    Q_EMIT videoLayerChanged(value);
                });

            p.compareVideoLayersObserver = feather_tk::ListObserver<int>::create(
                p.player->observeCompareVideoLayers(),
                [this](const std::vector<int>& value)
                {
                    Q_EMIT compareVideoLayersChanged(value);
                });

            p.currentVideoObserver = feather_tk::ListObserver<timeline::VideoData>::create(
                p.player->observeCurrentVideo(),
                [this](const std::vector<timeline::VideoData>& value)
                {
                    Q_EMIT currentVideoChanged(value);
                },
                feather_tk::ObserverAction::Suppress);

            p.audioDeviceObserver = feather_tk::ValueObserver<audio::DeviceID>::create(
                p.player->observeAudioDevice(),
                [this](const audio::DeviceID& value)
                {
                    Q_EMIT audioDeviceChanged(value);
                });

            p.volumeObserver = feather_tk::ValueObserver<float>::create(
                p.player->observeVolume(),
                [this](float value)
                {
                    Q_EMIT volumeChanged(value);
                });

            p.muteObserver = feather_tk::ValueObserver<bool>::create(
                p.player->observeMute(),
                [this](bool value)
                {
                    Q_EMIT muteChanged(value);
                });

            p.channelMuteObserver = feather_tk::ListObserver<bool>::create(
                p.player->observeChannelMute(),
                [this](const std::vector<bool>& value)
                {
                    Q_EMIT channelMuteChanged(value);
                });

            p.audioOffsetObserver = feather_tk::ValueObserver<double>::create(
                p.player->observeAudioOffset(),
                [this](double value)
                {
                    Q_EMIT audioOffsetChanged(value);
                });

            p.currentAudioObserver = feather_tk::ListObserver<timeline::AudioData>::create(
                p.player->observeCurrentAudio(),
                [this](const std::vector<timeline::AudioData>& value)
                {
                    Q_EMIT currentAudioChanged(value);
                });

            p.cacheOptionsObserver = feather_tk::ValueObserver<timeline::PlayerCacheOptions>::create(
                p.player->observeCacheOptions(),
                [this](const timeline::PlayerCacheOptions& value)
                {
                    Q_EMIT cacheOptionsChanged(value);
                });

            p.cacheInfoObserver = feather_tk::ValueObserver<timeline::PlayerCacheInfo>::create(
                p.player->observeCacheInfo(),
                [this](const timeline::PlayerCacheInfo& value)
                {
                    Q_EMIT cacheInfoChanged(value);
                });

            p.timer.reset(new QTimer);
            p.timer->setTimerType(Qt::PreciseTimer);
            connect(p.timer.get(), &QTimer::timeout, this, &PlayerObject::_timerCallback);
            p.timer->start(timeout);
        }

        PlayerObject::PlayerObject(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            _init(context, player);
        }

        PlayerObject::~PlayerObject()
        {}
        
        std::shared_ptr<feather_tk::Context> PlayerObject::context() const
        {
            return _p->player->getContext();
        }

        const std::shared_ptr<timeline::Player>& PlayerObject::player() const
        {
            return _p->player;
        }

        const std::shared_ptr<timeline::Timeline>& PlayerObject::timeline() const
        {
            return _p->player->getTimeline();
        }

        const file::Path& PlayerObject::path() const
        {
            return _p->player->getPath();
        }

        const file::Path& PlayerObject::audioPath() const
        {
            return _p->player->getAudioPath();
        }

        const timeline::PlayerOptions& PlayerObject::getPlayerOptions() const
        {
            return _p->player->getPlayerOptions();
        }

        const timeline::Options& PlayerObject::getOptions() const
        {
            return _p->player->getOptions();
        }

        const OTIO_NS::TimeRange& PlayerObject::timeRange() const
        {
            return _p->player->getTimeRange();
        }

        const io::Info& PlayerObject::ioInfo() const
        {
            return _p->player->getIOInfo();
        }

        double PlayerObject::defaultSpeed() const
        {
            return _p->player->getDefaultSpeed();
        }

        double PlayerObject::speed() const
        {
            return _p->player->getSpeed();
        }

        timeline::Playback PlayerObject::playback() const
        {
            return _p->player->getPlayback();
        }

        bool PlayerObject::isStopped() const
        {
            return _p->player->isStopped();
        }

        timeline::Loop PlayerObject::loop() const
        {
            return _p->player->getLoop();
        }

        const OTIO_NS::RationalTime& PlayerObject::currentTime() const
        {
            return _p->player->getCurrentTime();
        }

        const OTIO_NS::TimeRange& PlayerObject::inOutRange() const
        {
            return _p->player->getInOutRange();
        }

        const std::vector<std::shared_ptr<timeline::Timeline> >& PlayerObject::compare() const
        {
            return _p->player->getCompare();
        }

        timeline::CompareTime PlayerObject::compareTime() const
        {
            return _p->player->getCompareTime();
        }

        const io::Options& PlayerObject::ioOptions() const
        {
            return _p->player->getIOOptions();
        }

        int PlayerObject::videoLayer() const
        {
            return _p->player->getVideoLayer();
        }

        const std::vector<int>& PlayerObject::compareVideoLayers() const
        {
            return _p->player->getCompareVideoLayers();
        }

        const std::vector<timeline::VideoData>& PlayerObject::currentVideo() const
        {
            return _p->player->getCurrentVideo();
        }

        const audio::DeviceID& PlayerObject::audioDevice() const
        {
            return _p->player->getAudioDevice();
        }

        float PlayerObject::volume() const
        {
            return _p->player->getVolume();
        }

        bool PlayerObject::isMuted() const
        {
            return _p->player->isMuted();
        }

        const std::vector<bool>& PlayerObject::getChannelMute() const
        {
            return _p->player->getChannelMute();
        }

        double PlayerObject::audioOffset() const
        {
            return _p->player->getAudioOffset();
        }

        const std::vector<timeline::AudioData>& PlayerObject::currentAudio() const
        {
            return _p->player->getCurrentAudio();
        }

        const timeline::PlayerCacheOptions& PlayerObject::cacheOptions() const
        {
            return _p->player->getCacheOptions();
        }

        const timeline::PlayerCacheInfo& PlayerObject::cacheInfo() const
        {
            return _p->player->observeCacheInfo()->get();
        }

        void PlayerObject::setSpeed(double value)
        {
            _p->player->setSpeed(value);
        }

        void PlayerObject::setPlayback(timeline::Playback value)
        {
            _p->player->setPlayback(value);
        }

        void PlayerObject::stop()
        {
            _p->player->stop();
        }

        void PlayerObject::forward()
        {
            _p->player->forward();
        }

        void PlayerObject::reverse()
        {
            _p->player->reverse();
        }

        void PlayerObject::togglePlayback()
        {
            _p->player->setPlayback(
                timeline::Playback::Stop == _p->player->getPlayback() ?
                timeline::Playback::Forward :
                timeline::Playback::Stop);
        }

        void PlayerObject::setLoop(timeline::Loop value)
        {
            _p->player->setLoop(value);
        }

        void PlayerObject::seek(const OTIO_NS::RationalTime& value)
        {
            _p->player->seek(value);
        }

        void PlayerObject::timeAction(timeline::TimeAction value)
        {
            _p->player->timeAction(value);
        }

        void PlayerObject::gotoStart()
        {
            _p->player->gotoStart();
        }

        void PlayerObject::gotoEnd()
        {
            _p->player->gotoEnd();
        }

        void PlayerObject::framePrev()
        {
            _p->player->framePrev();
        }

        void PlayerObject::frameNext()
        {
            _p->player->frameNext();
        }

        void PlayerObject::setInOutRange(const OTIO_NS::TimeRange& value)
        {
            _p->player->setInOutRange(value);
        }

        void PlayerObject::setInPoint()
        {
            _p->player->setInPoint();
        }

        void PlayerObject::resetInPoint()
        {
            _p->player->resetInPoint();
        }

        void PlayerObject::setOutPoint()
        {
            _p->player->setOutPoint();
        }

        void PlayerObject::resetOutPoint()
        {
            _p->player->resetOutPoint();
        }

        void PlayerObject::setIOOptions(const io::Options& value)
        {
            _p->player->setIOOptions(value);
        }

        void PlayerObject::setCompare(const std::vector<std::shared_ptr<timeline::Timeline> >& value)
        {
            _p->player->setCompare(value);
        }

        void PlayerObject::setCompareTime(timeline::CompareTime value)
        {
            _p->player->setCompareTime(value);
        }

        void PlayerObject::setVideoLayer(int value)
        {
            _p->player->setVideoLayer(value);
        }

        void PlayerObject::setCompareVideoLayers(const std::vector<int>& value)
        {
            _p->player->setCompareVideoLayers(value);
        }

        void PlayerObject::setAudioDevice(const audio::DeviceID& value)
        {
            _p->player->setAudioDevice(value);
        }

        void PlayerObject::setVolume(float value)
        {
            _p->player->setVolume(value);
        }

        void PlayerObject::setMute(bool value)
        {
            _p->player->setMute(value);
        }

        void PlayerObject::setChannelMute(const std::vector<bool>& value)
        {
            _p->player->setChannelMute(value);
        }

        void PlayerObject::setAudioOffset(double value)
        {
            _p->player->setAudioOffset(value);
        }

        void PlayerObject::setCacheOptions(const timeline::PlayerCacheOptions& value)
        {
            _p->player->setCacheOptions(value);
        }

        void PlayerObject::_timerCallback()
        {
            if (_p && _p->player)
            {
                _p->player->tick();
            }
        }
    }
}
