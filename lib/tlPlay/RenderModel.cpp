// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/RenderModel.h>

#include <tlPlay/Settings.h>

namespace tl
{
    namespace play
    {
        struct RenderModel::Private
        {
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<Settings> settings;
            std::shared_ptr<dtk::ObservableValue<image::PixelType> > colorBuffer;
            std::shared_ptr<dtk::ObservableValue<timeline::ImageOptions> > imageOptions;
        };

        void RenderModel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<Settings>& settings)
        {
            TLRENDER_P();

            p.context = context;
            p.settings = settings;

            p.settings->setDefaultValue("Render/ColorBuffer", image::PixelType::RGBA_U8);
            p.colorBuffer = dtk::ObservableValue<image::PixelType>::create(
                p.settings->getValue<image::PixelType>("Render/ColorBuffer"));
            p.imageOptions = dtk::ObservableValue<timeline::ImageOptions>::create();
        }

        RenderModel::RenderModel() :
            _p(new Private)
        {}

        RenderModel::~RenderModel()
        {}

        std::shared_ptr<RenderModel> RenderModel::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<Settings>& settings)
        {
            auto out = std::shared_ptr<RenderModel>(new RenderModel);
            out->_init(context, settings);
            return out;
        }

        const timeline::ImageOptions& RenderModel::getImageOptions() const
        {
            return _p->imageOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<timeline::ImageOptions> > RenderModel::observeImageOptions() const
        {
            return _p->imageOptions;
        }

        void RenderModel::setImageOptions(const timeline::ImageOptions& value)
        {
            _p->imageOptions->setIfChanged(value);
        }

        image::PixelType RenderModel::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<dtk::IObservableValue<image::PixelType> > RenderModel::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void RenderModel::setColorBuffer(image::PixelType value)
        {
            _p->settings->setValue("Render/ColorBuffer", value);
            _p->colorBuffer->setIfChanged(value);
        }
    }
}
