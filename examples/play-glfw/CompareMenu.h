// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Menu.h>

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            class App;

            //! Compare menu.
            class CompareMenu : public ui::Menu
            {
                TLRENDER_NON_COPYABLE(CompareMenu);

            protected:
                void _init(
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<system::Context>&);

                CompareMenu();

            public:
                ~CompareMenu();

                static std::shared_ptr<CompareMenu> create(
                    const std::shared_ptr<App>&,
                    const std::shared_ptr<system::Context>&);

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
