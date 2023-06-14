// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "ViewMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct ViewMenu::Private
            {
            };

            void ViewMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                auto frameAction = ui::Action::create(context);
                frameAction->setText("Frame");
                frameAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(frameAction);
            }

            ViewMenu::ViewMenu() :
                _p(new Private)
            {}

            ViewMenu::~ViewMenu()
            {}

            std::shared_ptr<ViewMenu> ViewMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
                out->_init(app, context);
                return out;
            }
        }
    }
}
