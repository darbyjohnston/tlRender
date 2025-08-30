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
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            _actions["Stop"] = ftk::Action::create(
                "Stop",
                "PlaybackStop",
                ftk::Key::K,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->stop();
                    }
                });
            _actions["Stop"]->setTooltip("Stop playback.");

            _actions["Forward"] = ftk::Action::create(
                "Forward",
                "PlaybackForward",
                ftk::Key::L,
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

            _actions["Reverse"] = ftk::Action::create(
                "Reverse",
                "PlaybackReverse",
                ftk::Key::J,
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

            _actions["TogglePlayback"] = ftk::Action::create(
                "Toggle Playback",
                ftk::Key::Space,
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

            _actions["Start"] = ftk::Action::create(
                "Goto Start",
                "FrameStart",
                ftk::Key::Home,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->gotoStart();
                    }
                });
            _actions["Start"]->setTooltip("Go to the start frame.");

            _actions["Prev"] = ftk::Action::create(
                "Goto Previous",
                "FramePrev",
                ftk::Key::Left,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->framePrev();
                    }
                });
            _actions["Prev"]->setTooltip("Go to the previous frame.");

            _actions["Next"] = ftk::Action::create(
                "Goto Next",
                "FrameNext",
                ftk::Key::Right,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->frameNext();
                    }
                });
            _actions["Next"]->setTooltip("Go to the next frame.");

            _actions["End"] = ftk::Action::create(
                "Goto End",
                "FrameEnd",
                ftk::Key::End,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->gotoEnd();
                    }
                });
            _actions["End"]->setTooltip("Go to the end frame.");

            _actions["SetInPoint"] = ftk::Action::create(
                "Set In Point",
                ftk::Key::I,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->setInPoint();
                    }
                });
            _actions["SetInPoint"]->setTooltip("Set the playback in point.");

            _actions["ResetInPoint"] = ftk::Action::create(
                "Reset In Point",
                ftk::Key::I,
                static_cast<int>(ftk::KeyModifier::Shift),
                [this]
                {
                    if (_player)
                    {
                        _player->resetInPoint();
                    }
                });
            _actions["ResetInPoint"]->setTooltip("Reset the playback in point.");

            _actions["SetOutPoint"] = ftk::Action::create(
                "Set Out Point",
                ftk::Key::O,
                0,
                [this]
                {
                    if (_player)
                    {
                        _player->setOutPoint();
                    }
                });
            _actions["SetOutPoint"]->setTooltip("Set the playback out point.");

            _actions["ResetOutPoint"] = ftk::Action::create(
                "Reset Out Point",
                ftk::Key::O,
                static_cast<int>(ftk::KeyModifier::Shift),
                [this]
                {
                    if (_player)
                    {
                        _player->resetOutPoint();
                    }
                });
            _actions["ResetOutPoint"]->setTooltip("Reset the playback out point.");

            _playerObserver = ftk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<timeline::Player>& value)
                {
                    _player = value;

                    if (value)
                    {
                        _playbackObserver = ftk::ValueObserver<timeline::Playback>::create(
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
                    _actions["SetInPoint"]->setEnabled(value.get());
                    _actions["ResetInPoint"]->setEnabled(value.get());
                    _actions["SetOutPoint"]->setEnabled(value.get());
                    _actions["ResetOutPoint"]->setEnabled(value.get());
                });
        }

        PlaybackActions::~PlaybackActions()
        {
        }

        std::shared_ptr<PlaybackActions> PlaybackActions::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<PlaybackActions>(new PlaybackActions);
            out->_init(context, app);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ftk::Action> >& PlaybackActions::getActions() const
        {
            return _actions;
        }
    }
}