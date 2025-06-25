// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PlaybackActions.h"

#include "App.h"
#include "FilesModel.h"

namespace tl
{
    namespace play
    {
        void PlaybackActions::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            _actions["Stop"] = feather_tk::Action::create(
                "Stop",
                "PlaybackStop",
                feather_tk::Key::K,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->stop();
                    }
                });
            _actions["Stop"]->setTooltip("Stop playback.");

            _actions["Forward"] = feather_tk::Action::create(
                "Forward",
                "PlaybackForward",
                feather_tk::Key::L,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->forward();
                        _playback = timeline::Playback::Forward;
                    }
                });
            _actions["Forward"]->setTooltip("Start forward playback.");

            _actions["Reverse"] = feather_tk::Action::create(
                "Reverse",
                "PlaybackReverse",
                feather_tk::Key::J,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->reverse();
                        _playback = timeline::Playback::Reverse;
                    }
                });
            _actions["Reverse"]->setTooltip("Start reverse playback.");

            _actions["TogglePlayback"] = feather_tk::Action::create(
                "Toggle Playback",
                feather_tk::Key::Space,
                0,
                [this]
                {
                    if (_player)
                    {
                        if (_player->isStopped())
                        {
                            _player->setPlayback(_playback);
                        }
                        else
                        {
                            _player->stop();
                        }
                    }
                });

            _actions["Start"] = feather_tk::Action::create(
                "Goto Start",
                "FrameStart",
                feather_tk::Key::Home,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->gotoStart();
                    }
                });
            _actions["Start"]->setTooltip("Go to the start frame.");

            _actions["Prev"] = feather_tk::Action::create(
                "Goto Previous",
                "FramePrev",
                feather_tk::Key::Left,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->framePrev();
                    }
                });
            _actions["Prev"]->setTooltip("Go to the previous frame.");

            _actions["Next"] = feather_tk::Action::create(
                "Goto Next",
                "FrameNext",
                feather_tk::Key::Right,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->frameNext();
                    }
                });
            _actions["Next"]->setTooltip("Go to the next frame.");

            _actions["End"] = feather_tk::Action::create(
                "Goto End",
                "FrameEnd",
                feather_tk::Key::End,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->gotoEnd();
                    }
                });
            _actions["End"]->setTooltip("Go to the end frame.");

            _playerObserver = feather_tk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _player = value;

                    if (value)
                    {
                        _playbackObserver = feather_tk::ValueObserver<timeline::Playback>::create(
                            value->observePlayback(),
                            [this](timeline::Playback value)
                            {
                                _actions["Stop"]->setChecked(timeline::Playback::Stop == value);
                                _actions["Forward"]->setChecked(timeline::Playback::Forward == value);
                                _actions["Reverse"]->setChecked(timeline::Playback::Reverse == value);
                            });
                    }
                    else
                    {
                        _actions["Stop"]->setChecked(true);
                        _actions["Forward"]->setChecked(false);
                        _actions["Reverse"]->setChecked(false);

                        _playbackObserver.reset();
                    }

                    _actions["Stop"]->setEnabled(value.get());
                    _actions["Forward"]->setEnabled(value.get());
                    _actions["Reverse"]->setEnabled(value.get());
                    _actions["TogglePlayback"]->setEnabled(value.get());
                    _actions["Start"]->setEnabled(value.get());
                    _actions["Prev"]->setEnabled(value.get());
                    _actions["Next"]->setEnabled(value.get());
                    _actions["End"]->setEnabled(value.get());
                });
        }

        PlaybackActions::~PlaybackActions()
        {
        }

        std::shared_ptr<PlaybackActions> PlaybackActions::create(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<PlaybackActions>(new PlaybackActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<feather_tk::Action> >& PlaybackActions::getActions() const
        {
            return _actions;
        }
    }
}