// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Menu.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            class App;

            //! Playback menu.
            class PlaybackMenu : public ui::Menu
            {
                TLRENDER_NON_COPYABLE(PlaybackMenu);

            protected:
                void _init(
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<system::Context>&);

                PlaybackMenu();

            public:
                ~PlaybackMenu();

                static std::shared_ptr<PlaybackMenu> create(
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<system::Context>&);

            private:
                void _setPlayer(const std::shared_ptr<timeline::Player>&);
                void _playbackUpdate();

                TLRENDER_PRIVATE();
            };
        }
    }
}
