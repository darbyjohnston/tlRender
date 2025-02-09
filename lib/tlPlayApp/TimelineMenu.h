// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Menu.h>

namespace tl
{
    namespace play_app
    {
        class App;
        class MainWindow;

        //! Timeline menu.
        class TimelineMenu : public dtk::Menu
        {
            DTK_NON_COPYABLE(TimelineMenu);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            TimelineMenu();

        public:
            ~TimelineMenu();

            static std::shared_ptr<TimelineMenu> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&,
                const std::map<std::string, std::shared_ptr<dtk::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            void _thumbnailsSizeUpdate();

            DTK_PRIVATE();
        };
    }
}
