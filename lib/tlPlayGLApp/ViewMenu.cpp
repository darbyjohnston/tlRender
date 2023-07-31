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
            std::shared_ptr<ui::MenuItem> frameMenuItem;

            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
        };

        void ViewMenu::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.frameMenuItem = std::make_shared<ui::MenuItem>(
                "Frame",
                "ViewFrame",
                [this, mainWindowWeak](bool value)
                {
                    close();
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineViewport()->setFrameView(value);
                    }
                });
            addItem(p.frameMenuItem);

            auto item = std::make_shared<ui::MenuItem>(
                "Zoom 1:1",
                "ViewZoom1To1",
                [this, mainWindowWeak]
                {
                    close();
                if (auto mainWindow = mainWindowWeak.lock())
                {
                    mainWindow->getTimelineViewport()->viewZoom1To1();
                }
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

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getTimelineViewport()->observeFrameView(),
                [this](bool value)
                {
                    setItemChecked(_p->frameMenuItem, value);
                });
        }

        ViewMenu::ViewMenu() :
            _p(new Private)
        {}

        ViewMenu::~ViewMenu()
        {}

        std::shared_ptr<ViewMenu> ViewMenu::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewMenu>(new ViewMenu);
            out->_init(mainWindow, app, context, parent);
            return out;
        }
    }
}
