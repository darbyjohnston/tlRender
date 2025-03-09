// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/ViewActions.h>

#include <tlPlayApp/Models/ViewportModel.h>
#include <tlPlayApp/Widgets/Viewport.h>
#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <dtk/core/Format.h>

#include <sstream>

namespace tl
{
    namespace play
    {
        struct ViewActions::Private
        {
            std::vector<dtk::ImageType> colorBuffers;
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;

            std::shared_ptr<dtk::ValueObserver<KeyShortcutsSettings> > keyShortcutsSettingsObserver;
        };

        void ViewActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            DTK_P();

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.actions["Frame"] = dtk::Action::create(
                "Frame",
                "ViewFrame",
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setFrameView(value);
                    }
                });

            p.actions["ZoomReset"] = dtk::Action::create(
                "Zoom Reset",
                "ViewZoomReset",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomReset();
                    }
                });

            p.actions["ZoomIn"] = dtk::Action::create(
                "Zoom In",
                "ViewZoomIn",
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoomIn();
                    }
                });

            p.actions["ZoomOut"] = dtk::Action::create(
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
            p.actions["Red"] = dtk::Action::create(
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

            p.actions["Green"] = dtk::Action::create(
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

            p.actions["Blue"] = dtk::Action::create(
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

            p.actions["Alpha"] = dtk::Action::create(
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

            p.actions["MirrorHorizontal"] = dtk::Action::create(
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

            p.actions["MirrorVertical"] = dtk::Action::create(
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

            p.actions["MinifyNearest"] = dtk::Action::create(
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

            p.actions["MinifyLinear"] = dtk::Action::create(
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

            p.actions["MagnifyNearest"] = dtk::Action::create(
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

            p.actions["MagnifyLinear"] = dtk::Action::create(
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


            p.actions["FromFile"] = dtk::Action::create(
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

            p.actions["FullRange"] = dtk::Action::create(
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

            p.actions["LegalRange"] = dtk::Action::create(
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

            p.actions["AlphaBlendNone"] = dtk::Action::create(
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

            p.actions["AlphaBlendStraight"] = dtk::Action::create(
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

            p.actions["AlphaBlendPremultiplied"] = dtk::Action::create(
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

            p.colorBuffers.push_back(dtk::ImageType::RGBA_U8);
            p.colorBuffers.push_back(dtk::ImageType::RGBA_F16);
            p.colorBuffers.push_back(dtk::ImageType::RGBA_F32);
            for (size_t i = 0; i < p.colorBuffers.size(); ++i)
            {
                const dtk::ImageType imageType = p.colorBuffers[i];
                std::stringstream ss;
                ss << imageType;
                p.actions[ss.str()] = dtk::Action::create(
                    ss.str(),
                    [appWeak, imageType](bool value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getViewportModel()->setColorBuffer(imageType);
                        }
                    });
            }

            p.actions["HUD"] = dtk::Action::create(
                "HUD",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getViewportModel()->setHUD(value);
                    }
                });

            p.keyShortcutsSettingsObserver = dtk::ValueObserver<KeyShortcutsSettings>::create(
                app->getSettingsModel()->observeKeyShortcuts(),
                [this](const KeyShortcutsSettings& value)
                {
                    _keyShortcutsUpdate(value);
                });
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

        const std::vector<dtk::ImageType>& ViewActions::getColorBuffers() const
        {
            return _p->colorBuffers;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& ViewActions::getActions() const
        {
            return _p->actions;
        }

        void ViewActions::_keyShortcutsUpdate(const KeyShortcutsSettings& value)
        {
            DTK_P();
            const std::map<std::string, std::string> tooltips =
            {
                {
                    "Frame",
                    "Frame the view to fit the window.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "ZoomReset",
                    "Reset the view zoom to 1:1.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "ZoomIn",
                    "Zoom the view in.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "ZoomOut",
                    "Zoom the view out.\n"
                    "\n"
                    "Shortcut: {0}"
                },
                {
                    "HUD",
                    "Toggle the HUD (Heads Up Display).\n"
                    "\n"
                    "Shortcut: {0}"
                }
            };
            for (const auto& i : p.actions)
            {
                auto j = value.shortcuts.find(dtk::Format("View/{0}").arg(i.first));
                if (j != value.shortcuts.end())
                {
                    i.second->setShortcut(j->second.key);
                    i.second->setShortcutModifiers(j->second.modifiers);
                    const auto k = tooltips.find(i.first);
                    if (k != tooltips.end())
                    {
                        i.second->setTooltip(dtk::Format(k->second).
                            arg(dtk::getShortcutLabel(j->second.key, j->second.modifiers)));
                    }
                }
            }
        }
    }
}
