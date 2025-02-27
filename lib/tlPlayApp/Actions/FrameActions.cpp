// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/FrameActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct FrameActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void FrameActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Start"] = std::make_shared<dtk::Action>(
                "Go To Start",
                "TimeStart",
                dtk::Key::Home,
                0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->start();
                        }
                    }
                });
            p.actions["Start"]->toolTip = dtk::Format(
                "Go to the start frame\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Start"]->shortcut,
                    p.actions["Start"]->shortcutModifiers));

            p.actions["End"] = std::make_shared<dtk::Action>(
                "Go To End",
                "TimeEnd",
                dtk::Key::End,
                0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->end();
                        }
                    }
                });
            p.actions["End"]->toolTip = dtk::Format(
                "Go to the end frame\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["End"]->shortcut,
                    p.actions["End"]->shortcutModifiers));

            p.actions["Prev"] = std::make_shared<dtk::Action>(
                "Previous Frame",
                "FramePrev",
                dtk::Key::Left,
                0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->framePrev();
                        }
                    }
                });
            p.actions["Prev"]->toolTip = dtk::Format(
                "Go to the previous frame\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Prev"]->shortcut,
                    p.actions["Prev"]->shortcutModifiers));

            p.actions["PrevX10"] = std::make_shared<dtk::Action>(
                "Previous Frame X10",
                dtk::Key::Left,
                static_cast<int>(dtk::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FramePrevX10);
                        }
                    }
                });

            p.actions["PrevX100"] = std::make_shared<dtk::Action>(
                "Previous Frame X100",
                dtk::Key::Left,
                static_cast<int>(dtk::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FramePrevX100);
                        }
                    }
                });

            p.actions["Next"] = std::make_shared<dtk::Action>(
                "Next Frame",
                "FrameNext",
                dtk::Key::Right,
                0,
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->frameNext();
                        }
                    }
                });
            p.actions["Next"]->toolTip = dtk::Format(
                "Go to the next frame\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Next"]->shortcut,
                    p.actions["Next"]->shortcutModifiers));

            p.actions["NextX10"] = std::make_shared<dtk::Action>(
                "Next Frame X10",
                dtk::Key::Right,
                static_cast<int>(dtk::KeyModifier::Shift),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FrameNextX10);
                        }
                    }
                });

            p.actions["NextX100"] = std::make_shared<dtk::Action>(
                "Next Frame X100",
                dtk::Key::Right,
                static_cast<int>(dtk::KeyModifier::Control),
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::FrameNextX100);
                        }
                    }
                });

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["FocusCurrent"] = std::make_shared<dtk::Action>(
                "Focus Current Frame",
                dtk::Key::F,
                static_cast<int>(dtk::KeyModifier::Control),
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->focusCurrentFrame();
                    }
                });
        }

        FrameActions::FrameActions() :
            _p(new Private)
        {}

        FrameActions::~FrameActions()
        {}

        std::shared_ptr<FrameActions> FrameActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            auto out = std::shared_ptr<FrameActions>(new FrameActions);
            out->_init(context, app, mainWindow);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& FrameActions::getActions() const
        {
            return _p->actions;
        }
    }
}
