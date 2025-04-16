// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PlaybackActions.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace player
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
                    [this](bool)
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
                    [this](bool)
                    {
                        if (_player)
                        {
                            _player->forward();
                        }
                    });
                _actions["Forward"]->setTooltip("Start forward playback.");

                _actions["Reverse"] = dtk::Action::create(
                    "Reverse",
                    "PlaybackReverse",
                    dtk::Key::J,
                    0,
                    [this](bool)
                    {
                        if (_player)
                        {
                            _player->reverse();
                        }
                    });
                _actions["Reverse"]->setTooltip("Start reverse playback.");

                _playerObserver = dtk::ValueObserver<std::shared_ptr<timeline::Player> >::create(
                    app->observePlayer(),
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
                    });
            }

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
                return _actions;
            }
        }
    }
}
