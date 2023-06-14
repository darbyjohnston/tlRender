// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "WindowMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct WindowMenu::Private
            {
            };

            void WindowMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();
            }

            WindowMenu::WindowMenu() :
                _p(new Private)
            {}

            WindowMenu::~WindowMenu()
            {}

            std::shared_ptr<WindowMenu> WindowMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<WindowMenu>(new WindowMenu);
                out->_init(app, context);
                return out;
            }
        }
    }
}
