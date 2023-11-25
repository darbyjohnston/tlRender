// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Window.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Secondary window.
        class SecondaryWindow : public ui::Window
        {
            TLRENDER_NON_COPYABLE(SecondaryWindow);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            SecondaryWindow();

        public:
            virtual ~SecondaryWindow();

            static std::shared_ptr<SecondaryWindow> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
