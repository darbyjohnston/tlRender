// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewMenu.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/Viewport.h>

#include <tlPlay/ViewportModel.h>

namespace tl
{
    namespace play_app
    {
        struct ViewMenu::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<Menu> > menus;

            std::shared_ptr<dtk::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > hudObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::DisplayOptions> > displayOptionsObserver;
        };

        void ViewMenu::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            DTK_P();

            p.actions = actions;

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

            p.menus["MinifyFilter"] = addSubMenu("Minify Filter");
            p.menus["MinifyFilter"]->addItem(p.actions["MinifyNearest"]);
            p.menus["MinifyFilter"]->addItem(p.actions["MinifyLinear"]);

            p.menus["MagnifyFilter"] = addSubMenu("Magnify Filter");
            p.menus["MagnifyFilter"]->addItem(p.actions["MagnifyNearest"]);
            p.menus["MagnifyFilter"]->addItem(p.actions["MagnifyLinear"]);

            addDivider();
            addItem(p.actions["HUD"]);

            p.frameViewObserver = dtk::ValueObserver<bool>::create(
                mainWindow->getViewport()->observeFrameView(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["Frame"], value);
                });

            p.hudObserver = dtk::ValueObserver<bool>::create(
                mainWindow->getViewport()->observeHUD(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["HUD"], value);
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

                    _p->menus["MinifyFilter"]->setItemChecked(
                        _p->actions["MinifyNearest"],
                        dtk::ImageFilter::Nearest == value.imageFilters.minify);
                    _p->menus["MinifyFilter"]->setItemChecked(
                        _p->actions["MinifyLinear"],
                        dtk::ImageFilter::Linear == value.imageFilters.minify);

                    _p->menus["MagnifyFilter"]->setItemChecked(
                        _p->actions["MagnifyNearest"],
                        dtk::ImageFilter::Nearest == value.imageFilters.magnify);
                    _p->menus["MagnifyFilter"]->setItemChecked(
                        _p->actions["MagnifyLinear"],
                        dtk::ImageFilter::Linear == value.imageFilters.magnify);
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
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
            out->_init(context, app, mainWindow, actions, parent);
            return out;
        }
    }
}
