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

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::shared_ptr<Menu> thumbnailsSizeMenu;

            std::map<timeline::Playback, std::shared_ptr<ui::Action> > playbackItems;
            std::map<timeline::Loop, std::shared_ptr<ui::Action> > loopItems;
            std::map<int, std::shared_ptr<ui::Action> > thumbnailsSizeItems;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<observer::ValueObserver<timeline::Loop> > loopObserver;
            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<observer::ValueObserver<bool> > stopOnScrubObserver;
            std::shared_ptr<observer::ValueObserver<timelineui::ItemOptions> > itemOptionsObserver;
        };

        void PlaybackMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.mainWindow = mainWindow;

            p.actions = actions;
            addItem(p.actions["Stop"]);
            addItem(p.actions["Forward"]);
            addItem(p.actions["Reverse"]);
            addItem(p.actions["Toggle"]);
            addDivider();
            addItem(p.actions["JumpBack1s"]);
            addItem(p.actions["JumpBack10s"]);
            addItem(p.actions["JumpForward1s"]);
            addItem(p.actions["JumpForward10s"]);
            addDivider();
            addItem(p.actions["Loop"]);
            addItem(p.actions["Once"]);
            addItem(p.actions["PingPong"]);
            addDivider();
            addItem(p.actions["SetInPoint"]);
            addItem(p.actions["ResetInPoint"]);
            addItem(p.actions["SetOutPoint"]);
            addItem(p.actions["ResetOutPoint"]);
            addDivider();
            addItem(p.actions["FrameView"]);
            addItem(p.actions["StopOnScrub"]);
            addItem(p.actions["Thumbnails"]);
            p.thumbnailsSizeMenu = addSubMenu("Thumbnails Size");
            p.thumbnailsSizeMenu->addItem(p.actions["Thumbnails100"]);
            p.thumbnailsSizeMenu->addItem(p.actions["Thumbnails200"]);
            p.thumbnailsSizeMenu->addItem(p.actions["Thumbnails300"]);
            addItem(p.actions["Transitions"]);
            addItem(p.actions["Markers"]);

            p.playbackItems[timeline::Playback::Stop] = p.actions["Stop"];
            p.playbackItems[timeline::Playback::Forward] = p.actions["Forward"];
            p.playbackItems[timeline::Playback::Reverse] = p.actions["Reverse"];

            p.loopItems[timeline::Loop::Loop] = p.actions["Loop"];
            p.loopItems[timeline::Loop::Once] = p.actions["Once"];
            p.loopItems[timeline::Loop::PingPong] = p.actions["PingPong"];

            p.thumbnailsSizeItems[100] = p.actions["Thumbnails100"];
            p.thumbnailsSizeItems[200] = p.actions["Thumbnails200"];
            p.thumbnailsSizeItems[300] = p.actions["Thumbnails300"];

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
                    setItemChecked(_p->actions["FrameView"], value);
                });

            p.stopOnScrubObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineWidget()->observeStopOnScrub(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["StopOnScrub"], value);
                });

            p.itemOptionsObserver = observer::ValueObserver<timelineui::ItemOptions>::create(
                mainWindow->getTimelineWidget()->observeItemOptions(),
                [this](const timelineui::ItemOptions& value)
                {
                    setItemChecked(_p->actions["Thumbnails"], value.thumbnails);
                    _thumbnailsSizeUpdate();
                    setItemChecked(_p->actions["Transiitons"], value.showTransitions);
                    setItemChecked(_p->actions["Markers"], value.showMarkers);
                });
        }

        PlaybackMenu::PlaybackMenu() :
            _p(new Private)
        {}

        PlaybackMenu::~PlaybackMenu()
        {}

        std::shared_ptr<PlaybackMenu> PlaybackMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackMenu>(new PlaybackMenu);
            out->_init(actions, mainWindow, app, context, parent);
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
