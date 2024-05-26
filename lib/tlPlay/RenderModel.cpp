// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlay/RenderModel.h>

#include <tlPlay/Settings.h>

namespace tl
{
    namespace play
    {
        struct RenderModel::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<Settings> settings;
            std::shared_ptr<observer::Value<image::PixelType> > offscreenColorType;
            std::shared_ptr<observer::Value<timeline::ImageOptions> > imageOptions;
        };

        void RenderModel::_init(
            const std::shared_ptr<Settings>& settings,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;
            p.settings = settings;

            p.settings->setDefaultValue("Render/OffscreenColorType", image::PixelType::RGBA_U8);
            p.offscreenColorType = observer::Value<image::PixelType>::create(
                p.settings->getValue<image::PixelType>("Render/OffscreenColorType"));
            p.imageOptions = observer::Value<timeline::ImageOptions>::create();
        }

        RenderModel::RenderModel() :
            _p(new Private)
        {}

        RenderModel::~RenderModel()
        {}

        std::shared_ptr<RenderModel> RenderModel::create(
            const std::shared_ptr<Settings>& settings,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<RenderModel>(new RenderModel);
            out->_init(settings, context);
            return out;
        }

        const timeline::ImageOptions& RenderModel::getImageOptions() const
        {
            return _p->imageOptions->get();
        }

        std::shared_ptr<observer::IValue<timeline::ImageOptions> > RenderModel::observeImageOptions() const
        {
            return _p->imageOptions;
        }

        void RenderModel::setImageOptions(const timeline::ImageOptions& value)
        {
            _p->imageOptions->setIfChanged(value);
        }

        image::PixelType RenderModel::getOffscreenColorType() const
        {
            return _p->offscreenColorType->get();
        }

        std::shared_ptr<observer::IValue<image::PixelType> > RenderModel::observeOffscreenColorType() const
        {
            return _p->offscreenColorType;
        }

        void RenderModel::setOffscreenColorType(image::PixelType value)
        {
            _p->settings->setValue("Render/OffscreenColorType", value);
            _p->offscreenColorType->setIfChanged(value);
        }
    }
}
