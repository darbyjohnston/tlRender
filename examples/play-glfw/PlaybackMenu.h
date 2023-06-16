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

                void setFrameTimelineView(bool);
                void setStopOnScrub(bool);
                void setTimelineThumbnails(bool);

                void setFrameTimelineViewCallback(
                    const std::function<void(bool)>&);
                void setStopOnScrubCallback(
                    const std::function<void(bool)>&);
                void setTimelineThumbnailsCallback(
                    const std::function<void(bool)>&);

            private:
                void _setPlayer(const std::shared_ptr<timeline::Player>&);
                void _playbackUpdate();
                void _loopUpdate();

                TLRENDER_PRIVATE();
            };
        }
    }
}
