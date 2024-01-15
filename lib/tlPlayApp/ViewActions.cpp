// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace play_app
    {
        struct ViewActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void ViewActions::_init(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["Frame"] = std::make_shared<ui::Action>(
                "Frame",
                "ViewFrame",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineViewport()->setFrameView(value);
                    }
                });
            p.actions["Frame"]->toolTip = "Frame the view to fit the window";

            p.actions["Zoom1To1"] = std::make_shared<ui::Action>(
                "Zoom 1:1",
                "ViewZoom1To1",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineViewport()->viewZoom1To1();
                    }
                });
            p.actions["Zoom1To1"]->toolTip = "Set the view zoom to 1:1";

            p.actions["ZoomIn"] = std::make_shared<ui::Action>(
                "Zoom In",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineViewport()->viewZoomIn();
                    }
                });

            p.actions["ZoomOut"] = std::make_shared<ui::Action>(
                "Zoom Out",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getTimelineViewport()->viewZoomOut();
                    }
                });
        }

        ViewActions::ViewActions() :
            _p(new Private)
        {}

        ViewActions::~ViewActions()
        {}

        std::shared_ptr<ViewActions> ViewActions::create(
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ViewActions>(new ViewActions);
            out->_init(mainWindow, app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >& ViewActions::getActions() const
        {
            return _p->actions;
        }
    }
}
