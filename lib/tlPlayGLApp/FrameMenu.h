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

        //! Frame menu.
        class FrameMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(FrameMenu);

        protected:
            void _init(
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            FrameMenu();

        public:
            ~FrameMenu();

            static std::shared_ptr<FrameMenu> create(
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

        private:
            void _setPlayer(const std::shared_ptr<timeline::Player>&);

            TLRENDER_PRIVATE();
        };
    }
}

