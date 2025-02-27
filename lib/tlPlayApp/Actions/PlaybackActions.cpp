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
        };

        void PlaybackActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Stop"] = std::make_shared<dtk::Action>(
                "Stop",
                "PlaybackStop",
                dtk::Key::K,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Stop);
                        }
                    }
                });
            p.actions["Stop"]->toolTip = dtk::Format(
                "Stop playback\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Stop"]->shortcut,
                    p.actions["Stop"]->shortcutModifiers));

            p.actions["Forward"] = std::make_shared<dtk::Action>(
                "Forward",
                "PlaybackForward",
                dtk::Key::L,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Forward);
                        }
                    }
                });
            p.actions["Forward"]->toolTip = dtk::Format(
                "Forward playback\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Forward"]->shortcut,
                    p.actions["Forward"]->shortcutModifiers));

            p.actions["Reverse"] = std::make_shared<dtk::Action>(
                "Reverse",
                "PlaybackReverse",
                dtk::Key::J,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->setPlayback(timeline::Playback::Reverse);
                        }
                    }
                });
            p.actions["Reverse"]->toolTip = dtk::Format(
                "Reverse playback\n"
                "\n"
                "Shortcut: {0}").
                arg(dtk::getShortcutLabel(
                    p.actions["Reverse"]->shortcut,
                    p.actions["Reverse"]->shortcutModifiers));

            p.actions["Toggle"] = std::make_shared<dtk::Action>(
                "Toggle Playback",
                dtk::Key::Space,
                0,
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

            p.actions["JumpBack1s"] = std::make_shared<dtk::Action>(
                "Jump Back 1s",
                dtk::Key::J,
                static_cast<int>(dtk::KeyModifier::Shift),
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

            p.actions["JumpBack10s"] = std::make_shared<dtk::Action>(
                "Jump Back 10s",
                dtk::Key::J,
                static_cast<int>(dtk::KeyModifier::Control),
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

            p.actions["JumpForward1s"] = std::make_shared<dtk::Action>(
                "Jump Forward 1s",
                dtk::Key::L,
                static_cast<int>(dtk::KeyModifier::Shift),
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

            p.actions["JumpForward10s"] = std::make_shared<dtk::Action>(
                "Jump Forward 10s",
                dtk::Key::L,
                static_cast<int>(dtk::KeyModifier::Control),
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

            p.actions["Loop"] = std::make_shared<dtk::Action>(
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

            p.actions["Once"] = std::make_shared<dtk::Action>(
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

            p.actions["PingPong"] = std::make_shared<dtk::Action>(
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

            p.actions["SetInPoint"] = std::make_shared<dtk::Action>(
                "Set In Point",
                dtk::Key::I,
                0,
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

            p.actions["ResetInPoint"] = std::make_shared<dtk::Action>(
                "Reset In Point",
                dtk::Key::I,
                static_cast<int>(dtk::KeyModifier::Shift),
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

            p.actions["SetOutPoint"] = std::make_shared<dtk::Action>(
                "Set Out Point",
                dtk::Key::O,
                0,
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

            p.actions["ResetOutPoint"] = std::make_shared<dtk::Action>(
                "Reset Out Point",
                dtk::Key::O,
                static_cast<int>(dtk::KeyModifier::Shift),
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
    }
}
