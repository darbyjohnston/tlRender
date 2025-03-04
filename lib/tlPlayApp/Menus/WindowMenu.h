// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Menu.h>

namespace tl
{
    namespace play
    {
        class App;
        class MainWindow;
        class WindowActions;

        //! Window menu.
        class WindowMenu : public dtk::Menu
        {
            DTK_NON_COPYABLE(WindowMenu);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent);

            WindowMenu();

        public:
            ~WindowMenu();

            static std::shared_ptr<WindowMenu> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<MainWindow>&,
                const std::shared_ptr<WindowActions>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            DTK_PRIVATE();
        };
    }
}
