// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Menus/ViewMenu.h>

#include <tlPlayApp/Actions/ViewActions.h>
#include <tlPlayApp/Models/ViewportModel.h>
#include <tlPlayApp/Widgets/Viewport.h>
#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <sstream>

namespace tl
{
    namespace play
    {
        struct ViewMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<Menu> > menus;

            std::shared_ptr<dtk::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > hudObserver;
        };

        void ViewMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<ViewActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.actions = actions->getActions();

            addItem(p.actions["Frame"]);
            addItem(p.actions["ZoomReset"]);
            addItem(p.actions["ZoomIn"]);
            addItem(p.actions["ZoomOut"]);
            addDivider();
            addItem(p.actions["Red"]);
            addItem(p.actions["Green"]);
            addItem(p.actions["Blue"]);
            addItem(p.actions["Alpha"]);
            addDivider();
            addItem(p.actions["MirrorHorizontal"]);
            addItem(p.actions["MirrorVertical"]);
            addDivider();
            addItem(p.actions["HUD"]);

            p.frameViewObserver = dtk::ValueObserver<bool>::create(
                mainWindow->getViewport()->observeFrameView(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["Frame"], value);
                });

            p.displayOptionsObserver = dtk::ValueObserver<timeline::DisplayOptions>::create(
                app->getViewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    setItemChecked(
                        _p->actions["Red"],
                        dtk::ChannelDisplay::Red == value.channels);
                    setItemChecked(
                        _p->actions["Green"],
                        dtk::ChannelDisplay::Green == value.channels);
                    setItemChecked(
                        _p->actions["Blue"],
                        dtk::ChannelDisplay::Blue == value.channels);
                    setItemChecked(
                        _p->actions["Alpha"],
                        dtk::ChannelDisplay::Alpha == value.channels);

                    setItemChecked(
                        _p->actions["MirrorHorizontal"],
                        value.mirror.x);
                    setItemChecked(
                        _p->actions["MirrorVertical"],
                        value.mirror.y);
                });

            p.hudObserver = dtk::ValueObserver<bool>::create(
                app->getViewportModel()->observeHUD(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["HUD"], value);
                });
        }

        ViewMenu::ViewMenu() :
            _p(new Private)
        {}

        ViewMenu::~ViewMenu()
        {}

        std::shared_ptr<ViewMenu> ViewMenu::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<ViewActions>& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
            out->_init(context, app, mainWindow, actions, parent);
            return out;
        }
    }
}
