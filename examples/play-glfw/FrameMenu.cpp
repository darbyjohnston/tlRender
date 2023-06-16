// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "FrameMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct FrameMenu::Private
            {
                std::shared_ptr<timeline::Player> player;
                std::function<void(void)> focusCurrentFrameCallback;

                std::shared_ptr<observer::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            };

            void FrameMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                auto item = std::make_shared<ui::MenuItem>(
                    "Go To Start",
                    "TimeStart",
                    ui::Key::Home,
                    0,
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->start();
                        }
                        close();
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Go To End",
                    "TimeEnd",
                    ui::Key::End,
                    0,
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->end();
                        }
                        close();
                    });
                addItem(item);

                addDivider();

                item = std::make_shared<ui::MenuItem>(
                    "Previous Frame",
                    "FramePrev",
                    ui::Key::Left,
                    0,
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->framePrev();
                        }
                    close();
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Previous Frame X10",
                    ui::Key::Left,
                    static_cast<int>(ui::KeyModifier::Shift),
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->timeAction(timeline::TimeAction::FramePrevX10);
                        }
                        close();
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Previous Frame X100",
                    ui::Key::Left,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->timeAction(timeline::TimeAction::FramePrevX100);
                        }
                        close();
                    });
                addItem(item);

                addDivider();

                item = std::make_shared<ui::MenuItem>(
                    "Next Frame",
                    "FrameNext",
                    ui::Key::Right,
                    0,
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->frameNext();
                        }
                    close();
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Next Frame X10",
                    ui::Key::Right,
                    static_cast<int>(ui::KeyModifier::Shift),
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->timeAction(timeline::TimeAction::FrameNextX10);
                        }
                        close();
                    });
                addItem(item);

                item = std::make_shared<ui::MenuItem>(
                    "Next Frame X100",
                    ui::Key::Right,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        if (_p->player)
                        {
                            _p->player->timeAction(timeline::TimeAction::FrameNextX100);
                        }
                        close();
                    });
                addItem(item);

                addDivider();

                item = std::make_shared<ui::MenuItem>(
                    "Focus Current Frame",
                    ui::Key::F,
                    static_cast<int>(ui::KeyModifier::Control),
                    [this]
                    {
                        if (_p->focusCurrentFrameCallback)
                        {
                            _p->focusCurrentFrameCallback();
                        }
                        close();
                    });
                addItem(item);

                p.playerObserver = observer::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
                    [this](const std::shared_ptr<timeline::Player>& value)
                    {
                        _setPlayer(value);
                    });
            }

            FrameMenu::FrameMenu() :
                _p(new Private)
            {}

            FrameMenu::~FrameMenu()
            {}

            std::shared_ptr<FrameMenu> FrameMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<FrameMenu>(new FrameMenu);
                out->_init(app, context);
                return out;
            }

            void FrameMenu::setFocusCurrentFrameCallback(
                const std::function<void(void)>& value)
            {
                _p->focusCurrentFrameCallback = value;
            }

            void FrameMenu::_setPlayer(const std::shared_ptr<timeline::Player>& value)
            {
                TLRENDER_P();
                p.player = value;
            }
        }
    }
}
