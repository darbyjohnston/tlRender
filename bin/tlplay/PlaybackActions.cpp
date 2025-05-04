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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            _actions["Stop"] = dtk::Action::create(
                "Stop",
                "PlaybackStop",
                dtk::Key::K,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->stop();
                    }
                });
            _actions["Stop"]->setTooltip("Stop playback.");

            _actions["Forward"] = dtk::Action::create(
                "Forward",
                "PlaybackForward",
                dtk::Key::L,
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

            _actions["Reverse"] = dtk::Action::create(
                "Reverse",
                "PlaybackReverse",
                dtk::Key::J,
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

            _actions["TogglePlayback"] = dtk::Action::create(
                "Toggle Playback",
                dtk::Key::Space,
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

            _actions["Start"] = dtk::Action::create(
                "Goto Start",
                "FrameStart",
                dtk::Key::Home,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->gotoStart();
                    }
                });
            _actions["Start"]->setTooltip("Go to the start frame.");

            _actions["Prev"] = dtk::Action::create(
                "Goto Previous",
                "FramePrev",
                dtk::Key::Left,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->framePrev();
                    }
                });
            _actions["Prev"]->setTooltip("Go to the previous frame.");

            _actions["Next"] = dtk::Action::create(
                "Goto Next",
                "FrameNext",
                dtk::Key::Right,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->frameNext();
                    }
                });
            _actions["Next"]->setTooltip("Go to the next frame.");

            _actions["End"] = dtk::Action::create(
                "Goto End",
                "FrameEnd",
                dtk::Key::End,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->gotoEnd();
                    }
                });
            _actions["End"]->setTooltip("Go to the end frame.");

            _playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _player = value;

                    if (value)
                    {
                        _playbackObserver = dtk::ValueObserver<timeline::Playback>::create(
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<PlaybackActions>(new PlaybackActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& PlaybackActions::getActions() const
        {
            return _actions;
        }
    }
}