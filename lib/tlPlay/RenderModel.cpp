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
            std::shared_ptr<dtk::ObservableValue<dtk::ImageType> > colorBuffer;
            std::shared_ptr<dtk::ObservableValue<dtk::ImageOptions> > imageOptions;
        };

        void RenderModel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<Settings>& settings)
        {
            TLRENDER_P();

            p.context = context;
            p.settings = settings;

            p.settings->setDefaultValue("Render/ColorBuffer", dtk::ImageType::RGBA_U8);
            p.colorBuffer = dtk::ObservableValue<dtk::ImageType>::create(
                p.settings->getValue<dtk::ImageType>("Render/ColorBuffer"));
            p.imageOptions = dtk::ObservableValue<dtk::ImageOptions>::create();
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

        const dtk::ImageOptions& RenderModel::getImageOptions() const
        {
            return _p->imageOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::ImageOptions> > RenderModel::observeImageOptions() const
        {
            return _p->imageOptions;
        }

        void RenderModel::setImageOptions(const dtk::ImageOptions& value)
        {
            _p->imageOptions->setIfChanged(value);
        }

        dtk::ImageType RenderModel::getColorBuffer() const
        {
            return _p->colorBuffer->get();
        }

        std::shared_ptr<dtk::IObservableValue<dtk::ImageType> > RenderModel::observeColorBuffer() const
        {
            return _p->colorBuffer;
        }

        void RenderModel::setColorBuffer(dtk::ImageType value)
        {
            _p->settings->setValue("Render/ColorBuffer", value);
            _p->colorBuffer->setIfChanged(value);
        }
    }
}
