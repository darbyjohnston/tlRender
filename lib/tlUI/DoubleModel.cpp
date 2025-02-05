// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/DoubleModel.h>

#include <dtk/core/Math.h>

namespace tl
{
    namespace ui
    {
        struct DoubleModel::Private
        {
            std::shared_ptr<dtk::ObservableValue<double> > value;
            std::shared_ptr<dtk::ObservableValue<dtk::RangeD> > range;
            double step = 0.1;
            double largeStep = 1.0;
            std::shared_ptr<dtk::ObservableValue<bool> > hasDefaultValue;
            double defaultValue = 0.0;
        };

        void DoubleModel::_init(const std::shared_ptr<dtk::Context>&)
        {
            TLRENDER_P();
            p.value = dtk::ObservableValue<double>::create(0.0);
            p.range = dtk::ObservableValue<dtk::RangeD>::create(dtk::RangeD(0.0, 1.0));
            p.hasDefaultValue = dtk::ObservableValue<bool>::create(false);
        }

        DoubleModel::DoubleModel() :
            _p(new Private)
        {}

        DoubleModel::~DoubleModel()
        {}

        std::shared_ptr<DoubleModel> DoubleModel::create(
            const std::shared_ptr<dtk::Context>& context)
        {
            auto out = std::shared_ptr<DoubleModel>(new DoubleModel);
            out->_init(context);
            return out;
        }

        double DoubleModel::getValue() const
        {
            return _p->value->get();
        }

        void DoubleModel::setValue(double value)
        {
            TLRENDER_P();
            const dtk::RangeD& range = p.range->get();
            const double tmp = dtk::clamp(value, range.min(), range.max());
            _p->value->setIfChanged(tmp);
        }

        std::shared_ptr<dtk::IObservableValue<double> > DoubleModel::observeValue() const
        {
            return _p->value;
        }

        const dtk::RangeD& DoubleModel::getRange() const
        {
            return _p->range->get();
        }

        void DoubleModel::setRange(const dtk::RangeD& range)
        {
            TLRENDER_P();
            if (p.range->setIfChanged(range))
            {
                setValue(p.value->get());
            }
        }

        std::shared_ptr<dtk::IObservableValue<dtk::RangeD> > DoubleModel::observeRange() const
        {
            return _p->range;
        }

        double DoubleModel::getStep() const
        {
            return _p->step;
        }

        void DoubleModel::setStep(double value)
        {
            _p->step = value;
        }

        void DoubleModel::incrementStep()
        {
            TLRENDER_P();
            setValue(p.value->get() + p.step);
        }

        void DoubleModel::decrementStep()
        {
            TLRENDER_P();
            setValue(p.value->get() - p.step);
        }

        double DoubleModel::getLargeStep() const
        {
            return _p->largeStep;
        }

        void DoubleModel::setLargeStep(double value)
        {
            _p->largeStep = value;
        }

        void DoubleModel::incrementLargeStep()
        {
            TLRENDER_P();
            setValue(p.value->get() + p.largeStep);
        }

        void DoubleModel::decrementLargeStep()
        {
            TLRENDER_P();
            setValue(p.value->get() - p.largeStep);
        }

        bool DoubleModel::hasDefaultValue() const
        {
            return _p->hasDefaultValue->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > DoubleModel::observeHasDefaultValue() const
        {
            return _p->hasDefaultValue;
        }

        double DoubleModel::getDefaultValue() const
        {
            return _p->defaultValue;
        }

        void DoubleModel::setDefaultValue(double value)
        {
            _p->hasDefaultValue->setIfChanged(true);
            _p->defaultValue = value;
        }

        void DoubleModel::setDefaultValue()
        {
            setValue(_p->defaultValue);
        }

        void DoubleModel::clearDefaultValue()
        {
            _p->hasDefaultValue->setIfChanged(false);
        }
    }
}
