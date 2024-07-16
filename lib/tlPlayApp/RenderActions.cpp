// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/RenderActions.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/RenderModel.h>

namespace tl
{
    namespace play_app
    {
        struct RenderActions::Private
        {
            std::vector<image::PixelType> colorBuffers;
            std::map<std::string, std::shared_ptr<ui::Action> > actions;
        };

        void RenderActions::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["FromFile"] = std::make_shared<ui::Action>(
                "From File",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.videoLevels = timeline::InputVideoLevels::FromFile;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["FullRange"] = std::make_shared<ui::Action>(
                "Full Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.videoLevels = timeline::InputVideoLevels::FullRange;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["LegalRange"] = std::make_shared<ui::Action>(
                "Legal Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.videoLevels = timeline::InputVideoLevels::LegalRange;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendNone"] = std::make_shared<ui::Action>(
                "None",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.alphaBlend = timeline::AlphaBlend::None;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendStraight"] = std::make_shared<ui::Action>(
                "Straight",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.alphaBlend = timeline::AlphaBlend::Straight;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendPremultiplied"] = std::make_shared<ui::Action>(
                "Premultiplied",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.alphaBlend = timeline::AlphaBlend::Premultiplied;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.colorBuffers.push_back(image::PixelType::RGBA_U8);
            p.colorBuffers.push_back(image::PixelType::RGBA_F16);
            p.colorBuffers.push_back(image::PixelType::RGBA_F32);
            std::vector<std::pair<ui::Key, ui::KeyModifier> > colorBufferShortcuts =
            {
                std::make_pair(ui::Key::_8, ui::KeyModifier::Control),
                std::make_pair(ui::Key::_9, ui::KeyModifier::Control),
                std::make_pair(ui::Key::_0, ui::KeyModifier::Control)
            };
            for (size_t i = 0; i < p.colorBuffers.size(); ++i)
            {
                const image::PixelType pixelType = p.colorBuffers[i];
                std::stringstream ss;
                ss << pixelType;
                p.actions[ss.str()] = std::make_shared<ui::Action>(
                    ss.str(),
                    i < colorBufferShortcuts.size() ?
                        colorBufferShortcuts[i].first :
                        ui::Key::Unknown,
                    static_cast<int>(i < colorBufferShortcuts.size() ?
                        colorBufferShortcuts[i].second :
                        ui::KeyModifier::None),
                    [appWeak, pixelType](bool value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getRenderModel()->setColorBuffer(pixelType);
                        }
                    });
            }
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

        const std::vector<image::PixelType>& RenderActions::getColorBuffers() const
        {
            return _p->colorBuffers;
        }

        const std::map<std::string, std::shared_ptr<ui::Action> >& RenderActions::getActions() const
        {
            return _p->actions;
        }
    }
}
