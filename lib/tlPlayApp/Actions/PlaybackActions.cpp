// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/PlaybackActions.h>

#include <tlPlayApp/App.h>

#include <tlTimelineUI/TimelineWidget.h>

#include <dtk/core/Format.h>

namespace tl
{
    namespace play
    {
        struct PlaybackActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            timeline::Playback playbackPrev = timeline::Playback::Forward;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void PlaybackActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Stop"] = dtk::Action::create(
                "Stop",
                "PlaybackStop",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Stop);
                        }
                    }
                });

            p.actions["Forward"] = dtk::Action::create(
                "Forward",
                "PlaybackForward",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Forward);
                        }
                    }
                });

            p.actions["Reverse"] = dtk::Action::create(
                "Reverse",
                "PlaybackReverse",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Reverse);
                        }
                    }
                });

            p.actions["Toggle"] = dtk::Action::create(
                "Toggle Playback",
                [this, appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            const timeline::Playback playback = player->observePlayback()->get();
                            player->setPlayback(
                                timeline::Playback::Stop == playback ?
                                _p->playbackPrev :
                                timeline::Playback::Stop);
                            if (playback != timeline::Playback::Stop)
                            {
                                _p->playbackPrev = playback;
                            }
                        }
                    }
                });

            p.actions["JumpBack1s"] = dtk::Action::create(
                "Jump Back 1s",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::JumpBack1s);
                        }
                    }
                });

            p.actions["JumpBack10s"] = dtk::Action::create(
                "Jump Back 10s",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::JumpBack10s);
                        }
                    }
                });

            p.actions["JumpForward1s"] = dtk::Action::create(
                "Jump Forward 1s",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::JumpForward1s);
                        }
                    }
                });

            p.actions["JumpForward10s"] = dtk::Action::create(
                "Jump Forward 10s",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(timeline::TimeAction::JumpForward10s);
                        }
                    }
                });

            p.actions["Loop"] = dtk::Action::create(
                "Loop Playback",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setLoop(timeline::Loop::Loop);
                        }
                    }
                });

            p.actions["Once"] = dtk::Action::create(
                "Playback Once",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setLoop(timeline::Loop::Once);
                        }
                    }
                });

            p.actions["PingPong"] = dtk::Action::create(
                "Ping-Pong Playback",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setLoop(timeline::Loop::PingPong);
                        }
                    }
                });

            p.actions["SetInPoint"] = dtk::Action::create(
                "Set In Point",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setInPoint();
                        }
                    }
                });

            p.actions["ResetInPoint"] = dtk::Action::create(
                "Reset In Point",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->resetInPoint();
                        }
                    }
                });

            p.actions["SetOutPoint"] = dtk::Action::create(
                "Set Out Point",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setOutPoint();
                        }
                    }
                });

            p.actions["ResetOutPoint"] = dtk::Action::create(
                "Reset Out Point",
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->resetOutPoint();
                        }
                    }
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
                });
        }

        PlaybackActions::PlaybackActions() :
            _p(new Private)
        {}

        PlaybackActions::~PlaybackActions()
        {}

        std::shared_ptr<PlaybackActions> PlaybackActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<PlaybackActions>(new PlaybackActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& PlaybackActions::getActions() const
        {
            return _p->actions;
        }

        void PlaybackActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            auto i = value.shortcuts.find("Playback/Stop");
            if (i != value.shortcuts.end())
            {
                p.actions["Stop"]->setShortcut(i->second.key);
                p.actions["Stop"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Stop"]->setTooltip(dtk::Format(
                    "Stop playback.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/Forward");
            if (i != value.shortcuts.end())
            {
                p.actions["Forward"]->setShortcut(i->second.key);
                p.actions["Forward"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Forward"]->setTooltip(dtk::Format(
                    "Start forward playback.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/Reverse");
            if (i != value.shortcuts.end())
            {
                p.actions["Reverse"]->setShortcut(i->second.key);
                p.actions["Reverse"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Reverse"]->setTooltip(dtk::Format(
                    "Start reverse playback.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/Toggle");
            if (i != value.shortcuts.end())
            {
                p.actions["Toggle"]->setShortcut(i->second.key);
                p.actions["Toggle"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Toggle"]->setTooltip(dtk::Format(
                    "Toggle playback.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/JumpBack1s");
            if (i != value.shortcuts.end())
            {
                p.actions["JumpBack1s"]->setShortcut(i->second.key);
                p.actions["JumpBack1s"]->setShortcutModifiers(i->second.modifiers);
                p.actions["JumpBack1s"]->setTooltip(dtk::Format(
                    "Jump back 1 second.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/JumpBack10s");
            if (i != value.shortcuts.end())
            {
                p.actions["JumpBack10s"]->setShortcut(i->second.key);
                p.actions["JumpBack10s"]->setShortcutModifiers(i->second.modifiers);
                p.actions["JumpBack10s"]->setTooltip(dtk::Format(
                    "Jump back 10 seconds.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/JumpForward1s");
            if (i != value.shortcuts.end())
            {
                p.actions["JumpForward1s"]->setShortcut(i->second.key);
                p.actions["JumpForward1s"]->setShortcutModifiers(i->second.modifiers);
                p.actions["JumpForward1s"]->setTooltip(dtk::Format(
                    "Jump forward 1 second.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/JumpForward10s");
            if (i != value.shortcuts.end())
            {
                p.actions["JumpForward10s"]->setShortcut(i->second.key);
                p.actions["JumpForward10s"]->setShortcutModifiers(i->second.modifiers);
                p.actions["JumpForward10s"]->setTooltip(dtk::Format(
                    "Jump forward 10 seconds.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/Loop");
            if (i != value.shortcuts.end())
            {
                p.actions["Loop"]->setShortcut(i->second.key);
                p.actions["Loop"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Loop"]->setTooltip(dtk::Format(
                    "Loop playback.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/Once");
            if (i != value.shortcuts.end())
            {
                p.actions["Once"]->setShortcut(i->second.key);
                p.actions["Once"]->setShortcutModifiers(i->second.modifiers);
                p.actions["Once"]->setTooltip(dtk::Format(
                    "Playback once and then stop.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/PingPong");
            if (i != value.shortcuts.end())
            {
                p.actions["PingPong"]->setShortcut(i->second.key);
                p.actions["PingPong"]->setShortcutModifiers(i->second.modifiers);
                p.actions["PingPong"]->setTooltip(dtk::Format(
                    "Ping pong playback.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/SetInPoint");
            if (i != value.shortcuts.end())
            {
                p.actions["SetInPoint"]->setShortcut(i->second.key);
                p.actions["SetInPoint"]->setShortcutModifiers(i->second.modifiers);
                p.actions["SetInPoint"]->setTooltip(dtk::Format(
                    "Set the playback in point.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/ResetInPoint");
            if (i != value.shortcuts.end())
            {
                p.actions["ResetInPoint"]->setShortcut(i->second.key);
                p.actions["ResetInPoint"]->setShortcutModifiers(i->second.modifiers);
                p.actions["ResetInPoint"]->setTooltip(dtk::Format(
                    "Reset the playback in point.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/SetOutPoint");
            if (i != value.shortcuts.end())
            {
                p.actions["SetOutPoint"]->setShortcut(i->second.key);
                p.actions["SetOutPoint"]->setShortcutModifiers(i->second.modifiers);
                p.actions["SetOutPoint"]->setTooltip(dtk::Format(
                    "Set the playback out point.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
            i = value.shortcuts.find("Playback/ResetOutPoint");
            if (i != value.shortcuts.end())
            {
                p.actions["ResetOutPoint"]->setShortcut(i->second.key);
                p.actions["ResetOutPoint"]->setShortcutModifiers(i->second.modifiers);
                p.actions["ResetOutPoint"]->setTooltip(dtk::Format(
                    "Reset the playback out point.\n"
                    "\n"
                    "Shortcut: {0}").
                    arg(dtk::getShortcutLabel(i->second.key, i->second.modifiers)));
            }
        }
    }
}
