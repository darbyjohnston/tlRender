// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/ViewMenu.h>

#include <tlPlayApp/Actions/ViewActions.h>

namespace tl
{
    namespace play
    {
        struct ViewMenu::Private
        {
            std::map<std::string, std::shared_ptr<Menu> > menus;
        };

        void ViewMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            auto actions = viewActions->getActions();
            addItem(actions["Frame"]);
            addItem(actions["ZoomReset"]);
            addItem(actions["ZoomIn"]);
            addItem(actions["ZoomOut"]);
            addDivider();
            addItem(actions["Red"]);
            addItem(actions["Green"]);
            addItem(actions["Blue"]);
            addItem(actions["Alpha"]);
            addDivider();
            addItem(actions["MirrorHorizontal"]);
            addItem(actions["MirrorVertical"]);
            addDivider();
            addItem(actions["HUD"]);
        }

        ViewMenu::ViewMenu() :
            _p(new Private)
        {}

        ViewMenu::~ViewMenu()
        {}

        std::shared_ptr<ViewMenu> ViewMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
            out->_init(context, viewActions, parent);
            return out;
        }
    }
}
