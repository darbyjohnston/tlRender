// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "CompareMenu.h"

#include "App.h"

namespace tl
{
    namespace examples
    {
        namespace play_glfw
        {
            struct CompareMenu::Private
            {
            };

            void CompareMenu::_init(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                Menu::_init(context);
                TLRENDER_P();

                auto aAction = ui::Action::create(context);
                aAction->setText("A");
                aAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(aAction);

                auto bAction = ui::Action::create(context);
                bAction->setText("B");
                bAction->setClickedCallback(
                    [this]
                    {
                        close();
                    });
                addAction(bAction);
            }

            CompareMenu::CompareMenu() :
                _p(new Private)
            {}

            CompareMenu::~CompareMenu()
            {}

            std::shared_ptr<CompareMenu> CompareMenu::create(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
                out->_init(app, context);
                return out;
            }
        }
    }
}
