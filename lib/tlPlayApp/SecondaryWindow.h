// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Player.h>

#include <dtk/ui/Window.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Secondary window.
        class SecondaryWindow : public dtk::Window
        {
            DTK_NON_COPYABLE(SecondaryWindow);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<dtk::Window>&);

            SecondaryWindow();

        public:
            virtual ~SecondaryWindow();

            static std::shared_ptr<SecondaryWindow> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<dtk::Window>&);

            //! Set the view.
            void setView(
                const dtk::V2I& pos,
                double          zoom,
                bool            frame);

        private:
            DTK_PRIVATE();
        };
    }
}
