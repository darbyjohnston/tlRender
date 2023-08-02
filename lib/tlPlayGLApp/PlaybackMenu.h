// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_gl
    {
        class App;
        class MainWindow;

        //! Playback menu.
        class PlaybackMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(PlaybackMenu);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            PlaybackMenu();

        public:
            ~PlaybackMenu();

            static std::shared_ptr<PlaybackMenu> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            void _setPlayer(const std::shared_ptr<timeline::Player>&);
            void _playbackUpdate();
            void _loopUpdate();
            void _thumbnailsSizeUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
