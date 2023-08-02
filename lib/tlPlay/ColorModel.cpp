// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlay/ColorModel.h>

namespace tl
{
    namespace play
    {
        struct ColorModel::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::Value<timeline::ColorConfigOptions> > colorConfigOptions;
            std::shared_ptr<observer::Value<timeline::LUTOptions> > lutOptions;
            std::shared_ptr<observer::Value<timeline::ImageOptions> > imageOptions;
            std::shared_ptr<observer::Value<timeline::DisplayOptions> > displayOptions;
        };

        void ColorModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;

            p.colorConfigOptions = observer::Value<timeline::ColorConfigOptions>::create();
            p.lutOptions = observer::Value<timeline::LUTOptions>::create();
            p.imageOptions = observer::Value<timeline::ImageOptions>::create();
            p.displayOptions = observer::Value<timeline::DisplayOptions>::create();
        }

        ColorModel::ColorModel() :
            _p(new Private)
        {}

        ColorModel::~ColorModel()
        {}

        std::shared_ptr<ColorModel> ColorModel::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ColorModel>(new ColorModel);
            out->_init(context);
            return out;
        }

        const timeline::ColorConfigOptions& ColorModel::getColorConfigOptions() const
        {
            return _p->colorConfigOptions->get();
        }

        std::shared_ptr<observer::IValue<timeline::ColorConfigOptions> > ColorModel::observeColorConfigOptions() const
        {
            return _p->colorConfigOptions;
        }

        void ColorModel::setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            _p->colorConfigOptions->setIfChanged(value);
        }

        const timeline::LUTOptions& ColorModel::getLUTOptions() const
        {
            return _p->lutOptions->get();
        }

        std::shared_ptr<observer::IValue<timeline::LUTOptions> > ColorModel::observeLUTOptions() const
        {
            return _p->lutOptions;
        }

        void ColorModel::setLUTOptions(const timeline::LUTOptions& value)
        {
            _p->lutOptions->setIfChanged(value);
        }

        const timeline::ImageOptions& ColorModel::getImageOptions() const
        {
            return _p->imageOptions->get();
        }

        std::shared_ptr<observer::IValue<timeline::ImageOptions> > ColorModel::observeImageOptions() const
        {
            return _p->imageOptions;
        }

        void ColorModel::setImageOptions(const timeline::ImageOptions& value)
        {
            _p->imageOptions->setIfChanged(value);
        }

        const timeline::DisplayOptions& ColorModel::getDisplayOptions() const
        {
            return _p->displayOptions->get();
        }

        std::shared_ptr<observer::IValue<timeline::DisplayOptions> > ColorModel::observeDisplayOptions() const
        {
            return _p->displayOptions;
        }

        void ColorModel::setDisplayOptions(const timeline::DisplayOptions& value)
        {
            _p->displayOptions->setIfChanged(value);
        }
    }
}
