// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ViewMenu.h>

#include <tlPlayGLApp/App.h>

namespace tl
{
    namespace play_gl
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

            auto item = std::make_shared<ui::MenuItem>(
                "Frame",
                "ViewFrame",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Zoom 1:1",
                "ViewZoom1To1",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Zoom In",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);

            item = std::make_shared<ui::MenuItem>(
                "Zoom Out",
                [this]
                {
                    close();
                });
            addItem(item);
            setItemEnabled(item, false);
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
