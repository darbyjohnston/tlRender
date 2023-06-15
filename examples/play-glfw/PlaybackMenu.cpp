// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "PlaybackMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct PlaybackMenu::Private
            {
                std::map<timeline::Playback, std::shared_ptr<ui::MenuItem> > playbackItems;
                std::shared_ptr<timeline::Player> player;
                timeline::Playback playbackPrev = timeline::Playback::Forward;

                std::shared_ptr<observer::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
                std::shared_ptr<observer::ValueObserver<timeline::Playback> > playbackObserver;
            };

            void PlaybackMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                auto item = std::make_shared<ui::MenuItem>();
                item->text = "Stop";
                item->icon = "PlaybackStop";
                item->shortcut = ui::Key::K;
                item->checkable = true;
                item->checkedCallback = [this](bool value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Stop);
                    }
                    close();
                };
                addItem(item);
                p.playbackItems[timeline::Playback::Stop] = item;

                item = std::make_shared<ui::MenuItem>();
                item->text = "Forward";
                item->icon = "PlaybackForward";
                item->shortcut = ui::Key::L;
                item->checkable = true;
                item->checkedCallback = [this](bool value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Forward);
                    }
                    close();
                };
                addItem(item);
                p.playbackItems[timeline::Playback::Forward] = item;

                item = std::make_shared<ui::MenuItem>();
                item->text = "Reverse";
                item->icon = "PlaybackReverse";
                item->shortcut = ui::Key::J;
                item->checkable = true;
                item->checkedCallback = [this](bool value)
                {
                    if (_p->player)
                    {
                        _p->player->setPlayback(timeline::Playback::Reverse);
                    }
                    close();
                };
                addItem(item);
                p.playbackItems[timeline::Playback::Reverse] = item;

                item = std::make_shared<ui::MenuItem>();
                item->text = "Toggle Playback";
                item->shortcut = ui::Key::Space;
                item->callback = [this]
                {
                    if (_p->player)
                    {
                        const timeline::Playback playback = _p->player->observePlayback()->get();
                        _p->player->setPlayback(
                            timeline::Playback::Stop == playback ?
                            _p->playbackPrev :
                            timeline::Playback::Stop);
                        _p->playbackPrev = playback;
                    }
                    close();
                };
                addItem(item);

                _playbackUpdate();

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

            void PlaybackMenu::_setPlayer(const std::shared_ptr<timeline::Player>& value)
            {
                TLRENDER_P();
                p.playbackObserver.reset();
                p.player = value;
                if (p.player)
                {
                    p.playbackObserver = observer::ValueObserver<timeline::Playback>::create(
                        p.player->observePlayback(),
                        [this](timeline::Playback)
                        {
                            _playbackUpdate();
                        });
                }
            }

            void PlaybackMenu::_playbackUpdate()
            {
                TLRENDER_P();
                std::map<timeline::Playback, bool> playback;
                for (const auto& value : timeline::getPlaybackEnums())
                {
                    playback[value] = false;
                }
                playback[p.player ?
                    p.player->observePlayback()->get() :
                    timeline::Playback::Stop] = true;
                for (auto i : playback)
                {
                    setItemChecked(p.playbackItems[i.first], i.second);
                }
            }
        }
    }
}
