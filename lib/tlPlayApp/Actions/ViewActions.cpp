// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

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
        void ViewActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            IActions::_init(context, app, "View");

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            _actions["Frame"] = dtk::Action::create(
                "Frame",
                "ViewFrame",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setFrameView(value);
                    }
                });

            _actions["ZoomReset"] = dtk::Action::create(
                "Zoom Reset",
                "ViewZoomReset",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomReset();
                    }
                });

            _actions["ZoomIn"] = dtk::Action::create(
                "Zoom In",
                "ViewZoomIn",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomIn();
                    }
                });

            _actions["ZoomOut"] = dtk::Action::create(
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
            _actions["Red"] = dtk::Action::create(
                "Red Channel",
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

            _actions["Green"] = dtk::Action::create(
                "Green Channel",
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

            _actions["Blue"] = dtk::Action::create(
                "Blue Channel",
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

            _actions["Alpha"] = dtk::Action::create(
                "Alpha Channel",
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

            _actions["MirrorHorizontal"] = dtk::Action::create(
                "Mirror Horizontal",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.mirror.x = value;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            _actions["MirrorVertical"] = dtk::Action::create(
                "Mirror Vertical",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getViewportModel()->getDisplayOptions();
                        displayOptions.mirror.y = value;
                        app->getViewportModel()->setDisplayOptions(displayOptions);
                    }
                });

            _actions["MinifyNearest"] = dtk::Action::create(
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

            _actions["MinifyLinear"] = dtk::Action::create(
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

            _actions["MagnifyNearest"] = dtk::Action::create(
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

            _actions["MagnifyLinear"] = dtk::Action::create(
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


            _actions["FromFile"] = dtk::Action::create(
                "From File",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getViewportModel()->getImageOptions();
                        imageOptions.videoLevels = dtk::InputVideoLevels::FromFile;
                        app->getViewportModel()->setImageOptions(imageOptions);
                    }
                });

            _actions["FullRange"] = dtk::Action::create(
                "Full Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getViewportModel()->getImageOptions();
                        imageOptions.videoLevels = dtk::InputVideoLevels::FullRange;
                        app->getViewportModel()->setImageOptions(imageOptions);
                    }
                });

            _actions["LegalRange"] = dtk::Action::create(
                "Legal Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getViewportModel()->getImageOptions();
                        imageOptions.videoLevels = dtk::InputVideoLevels::LegalRange;
                        app->getViewportModel()->setImageOptions(imageOptions);
                    }
                });

            _actions["AlphaBlendNone"] = dtk::Action::create(
                "None",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getViewportModel()->getImageOptions();
                        imageOptions.alphaBlend = dtk::AlphaBlend::None;
                        app->getViewportModel()->setImageOptions(imageOptions);
                    }
                });

            _actions["AlphaBlendStraight"] = dtk::Action::create(
                "Straight",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getViewportModel()->getImageOptions();
                        imageOptions.alphaBlend = dtk::AlphaBlend::Straight;
                        app->getViewportModel()->setImageOptions(imageOptions);
                    }
                });

            _actions["AlphaBlendPremultiplied"] = dtk::Action::create(
                "Premultiplied",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getViewportModel()->getImageOptions();
                        imageOptions.alphaBlend = dtk::AlphaBlend::Premultiplied;
                        app->getViewportModel()->setImageOptions(imageOptions);
                    }
                });

            _colorBuffers.push_back(dtk::ImageType::RGBA_U8);
            _colorBuffers.push_back(dtk::ImageType::RGBA_F16);
            _colorBuffers.push_back(dtk::ImageType::RGBA_F32);
            for (size_t i = 0; i < _colorBuffers.size(); ++i)
            {
                const dtk::ImageType imageType = _colorBuffers[i];
                std::stringstream ss;
                ss << imageType;
                _actions[ss.str()] = dtk::Action::create(
                    ss.str(),
                    [appWeak, imageType](bool value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getViewportModel()->setColorBuffer(imageType);
                        }
                    });
            }

            _actions["HUD"] = dtk::Action::create(
                "HUD",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getViewportModel()->setHUD(value);
                    }
                });

            _tooltips =
            {
                { "Frame",  "Frame the view to fit the window." },
                { "ZoomReset", "Reset the view zoom to 1:1." },
                { "ZoomIn", "Zoom the view in." },
                { "ZoomOut", "Zoom the view out." },
                { "HUD", "Toggle the HUD (Heads Up Display)." }
            };

            _keyShortcutsUpdate(app->getSettingsModel()->getKeyShortcuts());
        }

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

        const std::vector<dtk::ImageType>& ViewActions::getColorBuffers() const
        {
            return _colorBuffers;
        }
    }
}
