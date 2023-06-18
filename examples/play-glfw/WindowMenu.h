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

                void setResizeCallback(const std::function<void(const imaging::Size&)>&);

                void setFullScreen(bool);
                void setFullScreenCallback(const std::function<void(bool)>&);

                void close() override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
