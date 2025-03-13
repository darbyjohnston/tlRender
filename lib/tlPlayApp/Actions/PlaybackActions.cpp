// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/PlaybackActions.h>

#include <tlPlayApp/App.h>

#include <tlTimelineUI/TimelineWidget.h>

namespace tl
{
    namespace play
    {
        struct PlaybackActions::Private
        {
            std::shared_ptr<timeline::Player> player;

            std::map<timeline::Playback, std::shared_ptr<dtk::Action> > playbackItems;
            std::map<timeline::Loop, std::shared_ptr<dtk::Action> > loopItems;

            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > playerObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::Loop> > loopObserver;
        };

        void PlaybackActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            IActions::_init(context, app, "Playback");
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            _actions["Stop"] = dtk::Action::create(
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

            _actions["Forward"] = dtk::Action::create(
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

            _actions["Reverse"] = dtk::Action::create(
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

            _actions["Toggle"] = dtk::Action::create(
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
                                _playbackPrev :
                                timeline::Playback::Stop);
                            if (playback != timeline::Playback::Stop)
                            {
                                _playbackPrev = playback;
                            }
                        }
                    }
                });

            _actions["JumpBack1s"] = dtk::Action::create(
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

            _actions["JumpBack10s"] = dtk::Action::create(
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

            _actions["JumpForward1s"] = dtk::Action::create(
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

            _actions["JumpForward10s"] = dtk::Action::create(
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

            _actions["Loop"] = dtk::Action::create(
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

            _actions["Once"] = dtk::Action::create(
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

            _actions["PingPong"] = dtk::Action::create(
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

            _actions["SetInPoint"] = dtk::Action::create(
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

            _actions["ResetInPoint"] = dtk::Action::create(
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

            _actions["SetOutPoint"] = dtk::Action::create(
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

            _actions["ResetOutPoint"] = dtk::Action::create(
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

            p.playbackItems[timeline::Playback::Stop] = _actions["Stop"];
            p.playbackItems[timeline::Playback::Forward] = _actions["Forward"];
            p.playbackItems[timeline::Playback::Reverse] = _actions["Reverse"];

            p.loopItems[timeline::Loop::Loop] = _actions["Loop"];
            p.loopItems[timeline::Loop::Once] = _actions["Once"];
            p.loopItems[timeline::Loop::PingPong] = _actions["PingPong"];

            _tooltips =
            {
                { "Stop", "Stop playback." },
                { "Forward", "Start forward playback." },
                { "Reverse", "Start reverse playback." },
                { "Toggle", "Toggle playback." },
                { "JumpBack1s", "Jump back 1 second." },
                { "JumpBack10s", "Jump back 10 seconds." },
                { "JumpForward1s", "Jump forward 1 second." },
                { "JumpForward10s", "Jump forward 10 seconds." },
                { "Loop", "Loop playback." },
                { "Once", "Playback once and then stop" },
                { "PingPong", "Ping pong playback." },
                { "SetInPoint", "Set the playback in point." },
                { "ResetInPoint", "Reet the playback in point." },
                { "SetOutPoint", "Set the playback out point." },
                { "ResetOutPoint", "Reet the playback out point." }
            };

            _keyShortcutsUpdate(app->getSettingsModel()->getKeyShortcuts());
            _playbackUpdate();
            _loopUpdate();

            p.playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _setPlayer(value);
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

        void PlaybackActions::_setPlayer(const std::shared_ptr<timeline::Player>& value)
        {
            DTK_P();
            p.playbackObserver.reset();
            p.loopObserver.reset();
            p.player = value;
            if (p.player)
            {
                p.playbackObserver = dtk::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback)
                    {
                        _playbackUpdate();
                    });
                p.loopObserver = dtk::ValueObserver<timeline::Loop>::create(
                    p.player->observeLoop(),
                    [this](timeline::Loop)
                    {
                        _loopUpdate();
                    });
            }
        }

        void PlaybackActions::_playbackUpdate()
        {
            DTK_P();
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
                p.playbackItems[i.first]->setChecked(i.second);
            }
        }

        void PlaybackActions::_loopUpdate()
        {
            DTK_P();
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
                p.loopItems[i.first]->setChecked(i.second);
            }
        }
    }
}
