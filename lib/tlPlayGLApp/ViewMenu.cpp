// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/ViewMenu.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/MainWindow.h>

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace play_gl
    {
        struct ViewMenu::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;

            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
        };

        void ViewMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.actions = actions;

            addItem(p.actions["Frame"]);
            addItem(p.actions["Zoom1To1"]);
            addItem(p.actions["ZoomIn"]);
            addItem(p.actions["ZoomOut"]);

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineViewport()->observeFrameView(),
                [this](bool value)
                {
                    setItemChecked(_p->actions["Frame"], value);
                });
        }

        ViewMenu::ViewMenu() :
            _p(new Private)
        {}

        ViewMenu::~ViewMenu()
        {}

        std::shared_ptr<ViewMenu> ViewMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }
    }
}
