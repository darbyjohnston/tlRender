// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUIApp/Window.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Secondary window.
        class SecondaryWindow : public ui_app::Window
        {
            TLRENDER_NON_COPYABLE(SecondaryWindow);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<ui_app::Window>&);

            SecondaryWindow();

        public:
            virtual ~SecondaryWindow();

            static std::shared_ptr<SecondaryWindow> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<ui_app::Window>&);

            //! Set the view.
            void setView(
                const tl::math::Vector2i& pos,
                double                    zoom,
                bool                      frame);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
