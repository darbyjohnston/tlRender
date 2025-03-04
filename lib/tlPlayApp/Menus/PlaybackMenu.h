// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <dtk/ui/Menu.h>

namespace tl
{
    namespace play
    {
        class App;
        class PlaybackActions;

        //! Playback menu.
        class PlaybackMenu : public dtk::Menu
        {
            DTK_NON_COPYABLE(PlaybackMenu);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<PlaybackActions>&,
                const std::shared_ptr<IWidget>& parent);

            PlaybackMenu();

        public:
            ~PlaybackMenu();

            static std::shared_ptr<PlaybackMenu> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<PlaybackActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            void _setPlayer(const std::shared_ptr<timeline::Player>&);
            void _playbackUpdate();
            void _loopUpdate();

            DTK_PRIVATE();
        };
    }
}
