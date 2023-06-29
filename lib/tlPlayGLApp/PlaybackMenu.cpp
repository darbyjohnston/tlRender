// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/PlaybackMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
    {
        struct PlaybackMenu::Private
        {
            std::shared_ptr<timeline::Player> player;

            std::map<timeline::Playback, std::shared_ptr<ui::MenuItem> > playbackItems;
            timeline::Playback playbackPrev = timeline::Playback::Forward;
            std::map<timeline::Loop, std::shared_ptr<ui::MenuItem> > loopItems;
            std::shared_ptr<ui::MenuItem> frameTimelineViewItem;
            std::shared_ptr<ui::MenuItem> stopOnScrubItem;
            std::shared_ptr<ui::MenuItem> timelineThumbnailsItem;
            std::function<void(bool)> frameTimelineViewCallback;
            std::function<void(bool)> stopOnScrubCallback;
            std::function<void(bool)> timelineThumbnailsCallback;

            std::shared_ptr<observer::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
        };

        void PlaybackMenu::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            p.playbackItems[timeline::Playback::Stop] = std::make_shared<ui::MenuItem>(
                "Stop",
                "PlaybackStop",
                ui::Key::K,
                0,
                [this](bool value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Stop);
                    }
                    close();
                });
            addItem(p.playbackItems[timeline::Playback::Stop]);

            p.playbackItems[timeline::Playback::Forward] = std::make_shared<ui::MenuItem>(
                "Forward",
                "PlaybackForward",
                ui::Key::L,
                0,
                [this](bool value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Forward);
                    }
                    close();
                });
            addItem(p.playbackItems[timeline::Playback::Forward]);

            p.playbackItems[timeline::Playback::Reverse] = std::make_shared<ui::MenuItem>(
                "Reverse",
                "PlaybackReverse",
                ui::Key::J,
                0,
                [this](bool value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Reverse);
                    }
                    close();
                });
            addItem(p.playbackItems[timeline::Playback::Reverse]);

            auto item = std::make_shared<ui::MenuItem>(
                "Toggle Playback",
                ui::Key::Space,
                0,
                [this]
                {
                    if (_p->player)
                    {
                        const timeline::Playback playback = _p->player->observePlayback()->get();
                        _p->player->setPlayback(
                            timeline::Playback::Stop == playback ?
                            _p->playbackPrev :
                            timeline::Playback::Stop);
                        if (playback != timeline::Playback::Stop)
                        {
                            _p->playbackPrev = playback;
                        }
                    }
                    close();
                });
            addItem(item);

            addDivider();

            p.loopItems[timeline::Loop::Loop] = std::make_shared<ui::MenuItem>(
                "Loop Playback",
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->setLoop(timeline::Loop::Loop);
                    }
                    close();
                });
            addItem(p.loopItems[timeline::Loop::Loop]);

            p.loopItems[timeline::Loop::Once] = std::make_shared<ui::MenuItem>(
                "Playback Once",
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->setLoop(timeline::Loop::Once);
                    }
                    close();
                });
            addItem(p.loopItems[timeline::Loop::Once]);

            p.loopItems[timeline::Loop::PingPong] = std::make_shared<ui::MenuItem>(
                "Ping-Pong Playback",
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->setLoop(timeline::Loop::PingPong);
                    }
                    close();
                });
            addItem(p.loopItems[timeline::Loop::PingPong]);

            addDivider();

            item = std::make_shared<ui::MenuItem>(
                "Set In Point",
                ui::Key::I,
                0,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->setInPoint();
                    }
                    close();
                });
            addItem(item);

            item = std::make_shared<ui::MenuItem>(
                "Reset In Point",
                ui::Key::I,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->resetInPoint();
                    }
                    close();
                });
            addItem(item);

            item = std::make_shared<ui::MenuItem>(
                "Set Out Point",
                ui::Key::O,
                0,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->setOutPoint();
                    }
                    close();
                });
            addItem(item);

            item = std::make_shared<ui::MenuItem>(
                "Reset Out Point",
                ui::Key::O,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->resetOutPoint();
                    }
                    close();
                });
            addItem(item);

            addDivider();

            p.frameTimelineViewItem = std::make_shared<ui::MenuItem>(
                "Frame Timeline View",
                [this](bool value)
                {
                    if (_p->frameTimelineViewCallback)
                    {
                        _p->frameTimelineViewCallback(value);
                    }
                    close();
                });
            addItem(p.frameTimelineViewItem);

            p.stopOnScrubItem = std::make_shared<ui::MenuItem>(
                "Stop When Scrubbing",
                [this](bool value)
                {
                    if (_p->stopOnScrubCallback)
                    {
                        _p->stopOnScrubCallback(value);
                    }
                    close();
                });
            addItem(p.stopOnScrubItem);

            p.timelineThumbnailsItem = std::make_shared<ui::MenuItem>(
                "Timeline Thumbnails",
                [this](bool value)
                {
                    if (_p->timelineThumbnailsCallback)
                    {
                        _p->timelineThumbnailsCallback(value);
                    }
                    close();
                });
            addItem(p.timelineThumbnailsItem);

            _playbackUpdate();
            _loopUpdate();

            p.playerObserver = observer::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _setPlayer(value);
                });
        }

        PlaybackMenu::PlaybackMenu() :
            _p(new Private)
        {}

        PlaybackMenu::~PlaybackMenu()
        {}

        std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
            out->_init(app, context);
            return out;
        }

        void PlaybackMenu::setFrameTimelineView(bool value)
        {
            setItemChecked(_p->frameTimelineViewItem, value);
        }

        void PlaybackMenu::setStopOnScrub(bool value)
        {
            setItemChecked(_p->stopOnScrubItem, value);
        }

        void PlaybackMenu::setTimelineThumbnails(bool value)
        {
            setItemChecked(_p->timelineThumbnailsItem, value);
        }

        void PlaybackMenu::setFrameTimelineViewCallback(
            const std::function<void(bool)>& value)
        {
            _p->frameTimelineViewCallback = value;
        }

        void PlaybackMenu::setStopOnScrubCallback(
            const std::function<void(bool)>& value)
        {
            _p->stopOnScrubCallback = value;
        }

        void PlaybackMenu::setTimelineThumbnailsCallback(
            const std::function<void(bool)>& value)
        {
            _p->timelineThumbnailsCallback = value;
        }

        void PlaybackMenu::_setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            TLRENDER_P();
            p.playbackObserver.reset();
            p.loopObserver.reset();
            p.player = value;
            if (p.player)
            {
                p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback)
                    {
                        _playbackUpdate();
                    });
                p.loopObserver = observer::ValueObserver<timeline::Loop>::create(
                    p.player->observeLoop(),
                    [this](timeline::Loop)
                    {
                        _loopUpdate();
                    });
            }
        }

        void PlaybackMenu::_playbackUpdate()
        {
            TLRENDER_P();
            std::map<timeline::Playback, bool> values;
            for (const auto& value : timeline::getPlaybackEnums())
            {
                values[value] = false;
            }
            values[p.player ?
                p.player->observePlayback()->get() :
                timeline::Playback::Stop] = true;
            for (auto i : values)
            {
                setItemChecked(p.playbackItems[i.first], i.second);
            }
        }

        void PlaybackMenu::_loopUpdate()
        {
            TLRENDER_P();
            std::map<timeline::Loop, bool> values;
            for (const auto& value : timeline::getLoopEnums())
            {
                values[value] = false;
            }
            values[p.player ?
                p.player->observeLoop()->get() :
                timeline::Loop::Loop] = true;
            for (auto i : values)
            {
                setItemChecked(p.loopItems[i.first], i.second);
            }
        }
    }
}
