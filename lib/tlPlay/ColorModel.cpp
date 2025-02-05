// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/ColorModel.h>

namespace tl
{
    namespace play
    {
        struct ColorModel::Private
        {
            std::weak_ptr<dtk::Context> context;
            std::shared_ptr<dtk::ObservableValue<timeline::OCIOOptions> > ocioOptions;
            std::shared_ptr<dtk::ObservableValue<timeline::LUTOptions> > lutOptions;
        };

        void ColorModel::_init(const std::shared_ptr<dtk::Context>& context)
        {
            DTK_P();

            p.context = context;

            p.ocioOptions = dtk::ObservableValue<timeline::OCIOOptions>::create();
            p.lutOptions = dtk::ObservableValue<timeline::LUTOptions>::create();
        }

        ColorModel::ColorModel() :
            _p(new Private)
        {}

        ColorModel::~ColorModel()
        {}

        std::shared_ptr<ColorModel> ColorModel::create(const std::shared_ptr<dtk::Context>& context)
        {
            auto out = std::shared_ptr<ColorModel>(new ColorModel);
            out->_init(context);
            return out;
        }

        const timeline::OCIOOptions& ColorModel::getOCIOOptions() const
        {
            return _p->ocioOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<timeline::OCIOOptions> > ColorModel::observeOCIOOptions() const
        {
            return _p->ocioOptions;
        }

        void ColorModel::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            _p->ocioOptions->setIfChanged(value);
        }

        const timeline::LUTOptions& ColorModel::getLUTOptions() const
        {
            return _p->lutOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<timeline::LUTOptions> > ColorModel::observeLUTOptions() const
        {
            return _p->lutOptions;
        }

        void ColorModel::setLUTOptions(const timeline::LUTOptions& value)
        {
            _p->lutOptions->setIfChanged(value);
        }
    }
}
