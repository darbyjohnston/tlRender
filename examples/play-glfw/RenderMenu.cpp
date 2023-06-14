// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "RenderMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct RenderMenu::Private
            {
            };

            void RenderMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();
            }

            RenderMenu::RenderMenu() :
                _p(new Private)
            {}

            RenderMenu::~RenderMenu()
            {}

            std::shared_ptr<RenderMenu> RenderMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<RenderMenu>(new RenderMenu);
                out->_init(app, context);
                return out;
            }
        }
    }
}
