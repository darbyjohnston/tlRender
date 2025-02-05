// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/FloatModel.h>

#include <dtk/core/Math.h>

namespace tl
{
    namespace ui
    {
        struct FloatModel::Private
        {
            std::shared_ptr<dtk::ObservableValue<float> > value;
            std::shared_ptr<dtk::ObservableValue<dtk::RangeF> > range;
            float step = .1F;
            float largeStep = 1.F;
            std::shared_ptr<dtk::ObservableValue<bool> > hasDefaultValue;
            float defaultValue = 0.F;
        };

        void FloatModel::_init(const std::shared_ptr<dtk::Context>&)
        {
            DTK_P();
            p.value = dtk::ObservableValue<float>::create(0.F);
            p.range = dtk::ObservableValue<dtk::RangeF>::create(dtk::RangeF(0.F, 1.F));
            p.hasDefaultValue = dtk::ObservableValue<bool>::create(false);
        }

        FloatModel::FloatModel() :
            _p(new Private)
        {}

        FloatModel::~FloatModel()
        {}

        std::shared_ptr<FloatModel> FloatModel::create(
            const std::shared_ptr<dtk::Context>& context)
        {
            auto out = std::shared_ptr<FloatModel>(new FloatModel);
            out->_init(context);
            return out;
        }

        float FloatModel::getValue() const
        {
            return _p->value->get();
        }

        void FloatModel::setValue(float value)
        {
            DTK_P();
            const dtk::RangeF& range = p.range->get();
            const float tmp = dtk::clamp(value, range.min(), range.max());
            _p->value->setIfChanged(tmp);
        }

        std::shared_ptr<dtk::IObservableValue<float> > FloatModel::observeValue() const
        {
            return _p->value;
        }

        const dtk::RangeF& FloatModel::getRange() const
        {
            return _p->range->get();
        }

        void FloatModel::setRange(const dtk::RangeF& range)
        {
            DTK_P();
            if (p.range->setIfChanged(range))
            {
                setValue(p.value->get());
            }
        }

        std::shared_ptr<dtk::IObservableValue<dtk::RangeF> > FloatModel::observeRange() const
        {
            return _p->range;
        }

        float FloatModel::getStep() const
        {
            return _p->step;
        }

        void FloatModel::setStep(float value)
        {
            _p->step = value;
        }

        void FloatModel::incrementStep()
        {
            DTK_P();
            setValue(p.value->get() + p.step);
        }

        void FloatModel::decrementStep()
        {
            DTK_P();
            setValue(p.value->get() - p.step);
        }

        float FloatModel::getLargeStep() const
        {
            return _p->largeStep;
        }

        void FloatModel::setLargeStep(float value)
        {
            _p->largeStep = value;
        }

        void FloatModel::incrementLargeStep()
        {
            DTK_P();
            setValue(p.value->get() + p.largeStep);
        }

        void FloatModel::decrementLargeStep()
        {
            DTK_P();
            setValue(p.value->get() - p.largeStep);
        }

        bool FloatModel::hasDefaultValue() const
        {
            return _p->hasDefaultValue->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > FloatModel::observeHasDefaultValue() const
        {
            return _p->hasDefaultValue;
        }

        float FloatModel::getDefaultValue() const
        {
            return _p->defaultValue;
        }

        void FloatModel::setDefaultValue(float value)
        {
            _p->hasDefaultValue->setIfChanged(true);
            _p->defaultValue = value;
        }

        void FloatModel::setDefaultValue()
        {
            setValue(_p->defaultValue);
        }

        void FloatModel::clearDefaultValue()
        {
            _p->hasDefaultValue->setIfChanged(false);
        }
    }
}
