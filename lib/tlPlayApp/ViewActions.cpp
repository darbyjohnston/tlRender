// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewActions.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/Viewport.h>

#include <tlPlay/ViewportModel.h>

namespace tl
{
    namespace play_app
    {
        struct ViewActions::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void ViewActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["Frame"] = std::make_shared<dtk::Action>(
                "Frame",
                "ViewFrame",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setFrameView(value);
                    }
                });
            p.actions["Frame"]->toolTip = "Frame the view to fit the window";

            p.actions["ZoomReset"] = std::make_shared<dtk::Action>(
                "Zoom Reset",
                "ViewZoomReset",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomReset();
                    }
                });
            p.actions["ZoomReset"]->toolTip = "Reset the view zoom to 1:1";

            p.actions["ZoomIn"] = std::make_shared<dtk::Action>(
                "Zoom In",
                "ViewZoomIn",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomIn();
                    }
                });

            p.actions["ZoomOut"] = std::make_shared<dtk::Action>(
                "Zoom Out",
                "ViewZoomOut",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomOut();
                    }
                });

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Red"] = std::make_shared<dtk::Action>(
                "Red Channel",
                dtk::Key::R,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            dtk::ChannelDisplay::Red :
                            dtk::ChannelDisplay::Color;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["Green"] = std::make_shared<dtk::Action>(
                "Green Channel",
                dtk::Key::G,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            dtk::ChannelDisplay::Green :
                            dtk::ChannelDisplay::Color;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["Blue"] = std::make_shared<dtk::Action>(
                "Blue Channel",
                dtk::Key::B,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            dtk::ChannelDisplay::Blue :
                            dtk::ChannelDisplay::Color;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["Alpha"] = std::make_shared<dtk::Action>(
                "Alpha Channel",
                dtk::Key::A,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            dtk::ChannelDisplay::Alpha :
                            dtk::ChannelDisplay::Color;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MirrorHorizontal"] = std::make_shared<dtk::Action>(
                "Mirror Horizontal",
                dtk::Key::H,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.mirror.x = value;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MirrorVertical"] = std::make_shared<dtk::Action>(
                "Mirror Vertical",
                dtk::Key::V,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.mirror.y = value;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MinifyNearest"] = std::make_shared<dtk::Action>(
                "Nearest",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.minify = dtk::ImageFilter::Nearest;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MinifyLinear"] = std::make_shared<dtk::Action>(
                "Linear",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.minify = dtk::ImageFilter::Linear;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MagnifyNearest"] = std::make_shared<dtk::Action>(
                "Nearest",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.magnify = dtk::ImageFilter::Nearest;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MagnifyLinear"] = std::make_shared<dtk::Action>(
                "Linear",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.imageFilters.magnify = dtk::ImageFilter::Linear;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["HUD"] = std::make_shared<dtk::Action>(
                "HUD",
                dtk::Key::H,
                static_cast<int>(dtk::KeyModifier::Control),
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setHUD(value);
                    }
                });
            p.actions["HUD"]->toolTip = "Toggle the HUD (Heads Up Display)";
        }

        ViewActions::ViewActions() :
            _p(new Private)
        {}

        ViewActions::~ViewActions()
        {}

        std::shared_ptr<ViewActions> ViewActions::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            auto out = std::shared_ptr<ViewActions>(new ViewActions);
            out->_init(context, app, mainWindow);
            return out;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& ViewActions::getActions() const
        {
            return _p->actions;
        }
    }
}
