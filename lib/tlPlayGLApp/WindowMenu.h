// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Menu.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! Window menu.
        class WindowMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(WindowMenu);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            WindowMenu();

        public:
            ~WindowMenu();

            static std::shared_ptr<WindowMenu> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            void close() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
