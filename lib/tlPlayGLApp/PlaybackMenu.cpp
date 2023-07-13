// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/PlaybackMenu.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/MainWindow.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play_gl
    {
        struct PlaybackMenu::Private
        {
            std::weak_ptr<MainWindow> mainWindow;
            std::shared_ptr<timeline::Player> player;

            std::map<timeline::Playback, std::shared_ptr<ui::MenuItem> > playbackItems;
            timeline::Playback playbackPrev = timeline::Playback::Forward;
            std::map<timeline::Loop, std::shared_ptr<ui::MenuItem> > loopItems;
            std::shared_ptr<ui::MenuItem> frameViewItem;
            std::shared_ptr<ui::MenuItem> stopOnScrubItem;
            std::shared_ptr<ui::MenuItem> thumbnailsItem;
            std::shared_ptr<Menu> thumbnailsSizeMenu;
            std::map<int, std::shared_ptr<ui::MenuItem> > thumbnailsSizeItems;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<observer::ValueObserver<bool> > stopOnScrubObserver;
            std::shared_ptr<observer::ValueObserver<timelineui::ItemOptions> > itemOptionsObserver;
        };

        void PlaybackMenu::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            Menu::_init(context);
            TLRENDER_P();

            p.mainWindow = mainWindow;

            p.playbackItems[timeline::Playback::Stop] = std::make_shared<ui::MenuItem>(
                "Stop",
                "PlaybackStop",
                ui::Key::K,
                0,
                [this](bool value)
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Stop);
                    }
                });
            addItem(p.playbackItems[timeline::Playback::Stop]);

            p.playbackItems[timeline::Playback::Forward] = std::make_shared<ui::MenuItem>(
                "Forward",
                "PlaybackForward",
                ui::Key::L,
                0,
                [this](bool value)
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Forward);
                    }
                });
            addItem(p.playbackItems[timeline::Playback::Forward]);

            p.playbackItems[timeline::Playback::Reverse] = std::make_shared<ui::MenuItem>(
                "Reverse",
                "PlaybackReverse",
                ui::Key::J,
                0,
                [this](bool value)
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Reverse);
                    }
                });
            addItem(p.playbackItems[timeline::Playback::Reverse]);

            auto item = std::make_shared<ui::MenuItem>(
                "Toggle Playback",
                ui::Key::Space,
                0,
                [this]
                {
                    close();
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
                });
            addItem(item);

            addDivider();

            p.loopItems[timeline::Loop::Loop] = std::make_shared<ui::MenuItem>(
                "Loop Playback",
                [this]
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setLoop(timeline::Loop::Loop);
                    }
                });
            addItem(p.loopItems[timeline::Loop::Loop]);

            p.loopItems[timeline::Loop::Once] = std::make_shared<ui::MenuItem>(
                "Playback Once",
                [this]
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setLoop(timeline::Loop::Once);
                    }
                });
            addItem(p.loopItems[timeline::Loop::Once]);

            p.loopItems[timeline::Loop::PingPong] = std::make_shared<ui::MenuItem>(
                "Ping-Pong Playback",
                [this]
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setLoop(timeline::Loop::PingPong);
                    }
                });
            addItem(p.loopItems[timeline::Loop::PingPong]);

            addDivider();

            item = std::make_shared<ui::MenuItem>(
                "Set In Point",
                ui::Key::I,
                0,
                [this]
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setInPoint();
                    }
                });
            addItem(item);

            item = std::make_shared<ui::MenuItem>(
                "Reset In Point",
                ui::Key::I,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->resetInPoint();
                    }
                });
            addItem(item);

            item = std::make_shared<ui::MenuItem>(
                "Set Out Point",
                ui::Key::O,
                0,
                [this]
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->setOutPoint();
                    }
                });
            addItem(item);

            item = std::make_shared<ui::MenuItem>(
                "Reset Out Point",
                ui::Key::O,
                static_cast<int>(ui::KeyModifier::Shift),
                [this]
                {
                    close();
                    if (_p->player)
                    {
                        _p->player->resetOutPoint();
                    }
                });
            addItem(item);

            addDivider();

            p.frameViewItem = std::make_shared<ui::MenuItem>(
                "Frame Timeline View",
                [this](bool value)
                {
                    close();
                    if (auto mainWindow = _p->mainWindow.lock())
                    {
                        mainWindow->getTimelineWidget()->setFrameView(value);
                    }
                });
            addItem(p.frameViewItem);

            p.stopOnScrubItem = std::make_shared<ui::MenuItem>(
                "Stop When Scrubbing",
                [this](bool value)
                {
                    close();
                    if (auto mainWindow = _p->mainWindow.lock())
                    {
                        mainWindow->getTimelineWidget()->setStopOnScrub(value);
                    }
                });
            addItem(p.stopOnScrubItem);

            p.thumbnailsItem = std::make_shared<ui::MenuItem>(
                "Timeline Thumbnails",
                [this](bool value)
                {
                    close();
                    if (auto mainWindow = _p->mainWindow.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnails = value;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });
            addItem(p.thumbnailsItem);

            p.thumbnailsSizeMenu = addSubMenu("Thumbnails Size");

            p.thumbnailsSizeItems[100] = std::make_shared<ui::MenuItem>(
                "Small",
                [this]
                {
                    close();
                    if (auto mainWindow = _p->mainWindow.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 100;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });
            p.thumbnailsSizeMenu->addItem(p.thumbnailsSizeItems[100]);

            p.thumbnailsSizeItems[200] = std::make_shared<ui::MenuItem>(
                "Medium",
                [this]
                {
                    close();
                    if (auto mainWindow = _p->mainWindow.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 200;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });
            p.thumbnailsSizeMenu->addItem(p.thumbnailsSizeItems[200]);

            p.thumbnailsSizeItems[300] = std::make_shared<ui::MenuItem>(
                "Large",
                [this]
                {
                    close();
                    if (auto mainWindow = _p->mainWindow.lock())
                    {
                        auto options = mainWindow->getTimelineWidget()->getItemOptions();
                        options.thumbnailHeight = 300;
                        options.waveformHeight = options.thumbnailHeight / 2;
                        mainWindow->getTimelineWidget()->setItemOptions(options);
                    }
                });
            p.thumbnailsSizeMenu->addItem(p.thumbnailsSizeItems[300]);

            _playbackUpdate();
            _loopUpdate();
            _thumbnailsSizeUpdate();

            p.playerObserver = observer::ListObserver<std::shared_ptr<timeline::Player> >::create(
                app->observeActivePlayers(),
                [this](const std::vector<std::shared_ptr<timeline::Player> >& value)
                {
                    _setPlayer(!value.empty() ? value[0] : nullptr);
                });

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeFrameView(),
                [this](bool value)
                {
                    setItemChecked(_p->frameViewItem, value);
                });

            p.stopOnScrubObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeStopOnScrub(),
                [this](bool value)
                {
                    setItemChecked(_p->stopOnScrubItem, value);
                });

            p.itemOptionsObserver = observer::ValueObserver<timelineui::ItemOptions>::create(
                mainWindow->getTimelineWidget()->observeItemOptions(),
                [this](const timelineui::ItemOptions& value)
                {
                    setItemChecked(_p->thumbnailsItem, value.thumbnails);
                    _thumbnailsSizeUpdate();
                });
        }

        PlaybackMenu::PlaybackMenu() :
            _p(new Private)
        {}

        PlaybackMenu::~PlaybackMenu()
        {}

        std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
            out->_init(mainWindow, app, context);
            return out;
        }

        void PlaybackMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            p.thumbnailsSizeMenu->close();
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

        void PlaybackMenu::_thumbnailsSizeUpdate()
        {
            TLRENDER_P();
            if (auto mainWindow = p.mainWindow.lock())
            {
                const auto options = mainWindow->getTimelineWidget()->getItemOptions();
                auto i = p.thumbnailsSizeItems.find(options.thumbnailHeight);
                if (i == p.thumbnailsSizeItems.end())
                {
                    i = p.thumbnailsSizeItems.begin();
                }
                for (auto item : p.thumbnailsSizeItems)
                {
                    const bool checked = item == *i;
                    p.thumbnailsSizeMenu->setItemChecked(item.second, checked);
                }
            }
        }
    }
}
