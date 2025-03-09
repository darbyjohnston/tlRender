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

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void FrameActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Start"] = dtk::Action::create(
                "Go To Start",
                "TimeStart",
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

            p.actions["End"] = dtk::Action::create(
                "Go To End",
                "TimeEnd",
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

            p.actions["Prev"] = dtk::Action::create(
                "Previous Frame",
                "FramePrev",
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

            p.actions["PrevX10"] = dtk::Action::create(
                "Previous Frame X10",
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

            p.actions["PrevX100"] = dtk::Action::create(
                "Previous Frame X100",
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

            p.actions["Next"] = dtk::Action::create(
                "Next Frame",
                "FrameNext",
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

            p.actions["NextX10"] = dtk::Action::create(
                "Next Frame X10",
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

            p.actions["NextX100"] = dtk::Action::create(
                "Next Frame X100",
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
            p.actions["FocusCurrent"] = dtk::Action::create(
                "Focus Current Frame",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->focusCurrentFrame();
                    }
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
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

        void FrameActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            auto i = value.shortcuts.find("Frame/Start");
            if (i != value.shortcuts.end())
            {
                p.actions["Start"]->setShortcut(i->second.key);
                p.actions["Start"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Start"]->setTooltip(dtk::Format(
                    "Go to the start frame.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/End");
            if (i != value.shortcuts.end())
            {
                p.actions["End"]->setShortcut(i->second.key);
                p.actions["End"]->setShortcutModifiers(i->second.modifiers);
                p.actions["End"]->setTooltip(dtk::Format(
                    "Go to the end frame.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/Prev");
            if (i != value.shortcuts.end())
            {
                p.actions["Prev"]->setShortcut(i->second.key);
                p.actions["Prev"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Prev"]->setTooltip(dtk::Format(
                    "Go to the previous frame.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/PrevX10");
            if (i != value.shortcuts.end())
            {
                p.actions["PrevX10"]->setShortcut(i->second.key);
                p.actions["PrevX10"]->setShortcutModifiers(i->second.modifiers);
                p.actions["PrevX10"]->setTooltip(dtk::Format(
                    "Go to the previous frame X10.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/PrevX100");
            if (i != value.shortcuts.end())
            {
                p.actions["PrevX100"]->setShortcut(i->second.key);
                p.actions["PrevX100"]->setShortcutModifiers(i->second.modifiers);
                p.actions["PrevX100"]->setTooltip(dtk::Format(
                    "Go to the previous frame X100.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/Next");
            if (i != value.shortcuts.end())
            {
                p.actions["Next"]->setShortcut(i->second.key);
                p.actions["Next"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Next"]->setTooltip(dtk::Format(
                    "Go to the next frame.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/NextX10");
            if (i != value.shortcuts.end())
            {
                p.actions["NextX10"]->setShortcut(i->second.key);
                p.actions["NextX10"]->setShortcutModifiers(i->second.modifiers);
                p.actions["NextX10"]->setTooltip(dtk::Format(
                    "Go to the next frame X10.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/NextX100");
            if (i != value.shortcuts.end())
            {
                p.actions["NextX100"]->setShortcut(i->second.key);
                p.actions["NextX100"]->setShortcutModifiers(i->second.modifiers);
                p.actions["NextX100"]->setTooltip(dtk::Format(
                    "Go to the next frame X100.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Frame/FocusCurrent");
            if (i != value.shortcuts.end())
            {
                p.actions["FocusCurrent"]->setShortcut(i->second.key);
                p.actions["FocusCurrent"]->setShortcutModifiers(i->second.modifiers);
                p.actions["FocusCurrent"]->setTooltip(dtk::Format(
                    "Set the keyboard focus to the current frame editor.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
        }
    }
}
