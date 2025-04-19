// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <dtk/ui/Action.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;

            //! Playback actions.
            class PlaybackActions : public std::enable_shared_from_this<PlaybackActions>
            {
                DTK_NON_COPYABLE(PlaybackActions);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                PlaybackActions() = default;

            public:
                ~PlaybackActions();

                static std::shared_ptr<PlaybackActions> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                const std::map<std::string, std::shared_ptr<dtk::Action> >& getActions() const;

            private:
                std::map<std::string, std::shared_ptr<dtk::Action> > _actions;
                std::shared_ptr<timeline::Player> _player;
                timeline::Playback _playback = timeline::Playback::Forward;
                std::shared_ptr<dtk::ValueObserver<std::shared_ptr<timeline::Player> > > _playerObserver;
                std::shared_ptr<dtk::ValueObserver<timeline::Playback> > _playbackObserver;
            };
        }
    }
}
