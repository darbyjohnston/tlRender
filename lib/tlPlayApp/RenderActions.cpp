// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayApp/RenderActions.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ColorModel.h>

namespace tl
{
    namespace play_app
    {
        struct RenderActions::Private
        {
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void RenderActions::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["Red"] = std::make_shared<ui::Action>(
                "Red Channel",
                ui::Key::R,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            timeline::Channels::Red :
                            timeline::Channels::Color;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["Green"] = std::make_shared<ui::Action>(
                "Green Channel",
                ui::Key::G,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            timeline::Channels::Green :
                            timeline::Channels::Color;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["Blue"] = std::make_shared<ui::Action>(
                "Blue Channel",
                ui::Key::B,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            timeline::Channels::Blue :
                            timeline::Channels::Color;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["Alpha"] = std::make_shared<ui::Action>(
                "Alpha Channel",
                ui::Key::A,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.channels = value ?
                            timeline::Channels::Alpha :
                            timeline::Channels::Color;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MirrorHorizontal"] = std::make_shared<ui::Action>(
                "Mirror Horizontal",
                ui::Key::H,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.mirror.x = value;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MirrorVertical"] = std::make_shared<ui::Action>(
                "Mirror Vertical",
                ui::Key::V,
                0,
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.mirror.y = value;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["FromFile"] = std::make_shared<ui::Action>(
                "From File",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getColorModel()->getImageOptions();
                        imageOptions.videoLevels = timeline::InputVideoLevels::FromFile;
                        app->getColorModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["FullRange"] = std::make_shared<ui::Action>(
                "Full Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getColorModel()->getImageOptions();
                        imageOptions.videoLevels = timeline::InputVideoLevels::FullRange;
                        app->getColorModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["LegalRange"] = std::make_shared<ui::Action>(
                "Legal Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getColorModel()->getImageOptions();
                        imageOptions.videoLevels = timeline::InputVideoLevels::LegalRange;
                        app->getColorModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendNone"] = std::make_shared<ui::Action>(
                "None",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getColorModel()->getImageOptions();
                        imageOptions.alphaBlend = timeline::AlphaBlend::None;
                        app->getColorModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendStraight"] = std::make_shared<ui::Action>(
                "Straight",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getColorModel()->getImageOptions();
                        imageOptions.alphaBlend = timeline::AlphaBlend::Straight;
                        app->getColorModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendPremultiplied"] = std::make_shared<ui::Action>(
                "Premultiplied",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getColorModel()->getImageOptions();
                        imageOptions.alphaBlend = timeline::AlphaBlend::Premultiplied;
                        app->getColorModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["MinifyNearest"] = std::make_shared<ui::Action>(
                "Nearest",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.imageFilters.minify = timeline::ImageFilter::Nearest;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MinifyLinear"] = std::make_shared<ui::Action>(
                "Linear",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.imageFilters.minify = timeline::ImageFilter::Linear;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MagnifyNearest"] = std::make_shared<ui::Action>(
                "Nearest",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.imageFilters.magnify = timeline::ImageFilter::Nearest;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });

            p.actions["MagnifyLinear"] = std::make_shared<ui::Action>(
                "Linear",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto displayOptions = app->getColorModel()->getDisplayOptions();
                        displayOptions.imageFilters.magnify = timeline::ImageFilter::Linear;
                        app->getColorModel()->setDisplayOptions(displayOptions);
                    }
                });
        }

        RenderActions::RenderActions() :
            _p(new Private)
        {}

        RenderActions::~RenderActions()
        {}

        std::shared_ptr<RenderActions> RenderActions::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<RenderActions>(new RenderActions);
            out->_init(app, context);
            return out;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >& RenderActions::getActions() const
        {
            return _p->actions;
        }
    }
}
