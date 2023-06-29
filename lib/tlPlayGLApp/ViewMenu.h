// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Menu.h>

namespace tl
{
    namespace play_gl
    {
        class App;

        //! View menu.
        class ViewMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(ViewMenu);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            ViewMenu();

        public:
            ~ViewMenu();

            static std::shared_ptr<ViewMenu> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
