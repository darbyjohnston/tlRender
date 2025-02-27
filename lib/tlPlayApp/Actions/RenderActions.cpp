// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Actions/RenderActions.h>

#include <tlPlayApp/Models/ColorModel.h>
#include <tlPlayApp/Models/RenderModel.h>
#include <tlPlayApp/App.h>

#include <sstream>

namespace tl
{
    namespace play
    {
        struct RenderActions::Private
        {
            std::vector<dtk::ImageType> colorBuffers;
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
        };

        void RenderActions::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            DTK_P();

            auto appWeak = std::weak_ptr<App>(app);
            p.actions["FromFile"] = std::make_shared<dtk::Action>(
                "From File",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.videoLevels = dtk::InputVideoLevels::FromFile;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["FullRange"] = std::make_shared<dtk::Action>(
                "Full Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.videoLevels = dtk::InputVideoLevels::FullRange;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["LegalRange"] = std::make_shared<dtk::Action>(
                "Legal Range",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.videoLevels = dtk::InputVideoLevels::LegalRange;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendNone"] = std::make_shared<dtk::Action>(
                "None",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.alphaBlend = dtk::AlphaBlend::None;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendStraight"] = std::make_shared<dtk::Action>(
                "Straight",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.alphaBlend = dtk::AlphaBlend::Straight;
                        app->getRenderModel()->setImageOptions(imageOptions);
                    }
                });

            p.actions["AlphaBlendPremultiplied"] = std::make_shared<dtk::Action>(
                "Premultiplied",
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto imageOptions = app->getRenderModel()->getImageOptions();
                        imageOptions.alphaBlend = dtk::AlphaBlend::Premultiplied;
                        app->getRenderModel()->setImageOptions(imageOptions);
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
                p.actions[ss.str()] = std::make_shared<dtk::Action>(
                    ss.str(),
                    [appWeak, imageType](bool value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getRenderModel()->setColorBuffer(imageType);
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app)
        {
            auto out = std::shared_ptr<RenderActions>(new RenderActions);
            out->_init(context, app);
            return out;
        }

        const std::vector<dtk::ImageType>& RenderActions::getColorBuffers() const
        {
            return _p->colorBuffers;
        }

        const std::map<std::string, std::shared_ptr<dtk::Action> >& RenderActions::getActions() const
        {
            return _p->actions;
        }
    }
}
