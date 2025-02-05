// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/IntModel.h>

#include <dtk/core/Math.h>

namespace tl
{
    namespace ui
    {
        struct IntModel::Private
        {
            std::shared_ptr<dtk::ObservableValue<int> > value;
            std::shared_ptr<dtk::ObservableValue<dtk::RangeI> > range;
            int step = 1;
            int largeStep = 10;
            std::shared_ptr<dtk::ObservableValue<bool> > hasDefaultValue;
            int defaultValue = 0;
        };

        void IntModel::_init(const std::shared_ptr<dtk::Context>&)
        {
            DTK_P();
            p.value = dtk::ObservableValue<int>::create(0);
            p.range = dtk::ObservableValue<dtk::RangeI>::create(dtk::RangeI(0, 100));
            p.hasDefaultValue = dtk::ObservableValue<bool>::create(false);
        }

        IntModel::IntModel() :
            _p(new Private)
        {}

        IntModel::~IntModel()
        {}

        std::shared_ptr<IntModel> IntModel::create(
            const std::shared_ptr<dtk::Context>& context)
        {
            auto out = std::shared_ptr<IntModel>(new IntModel);
            out->_init(context);
            return out;
        }

        int IntModel::getValue() const
        {
            return _p->value->get();
        }

        void IntModel::setValue(int value)
        {
            DTK_P();
            const dtk::RangeI& range = p.range->get();
            const int tmp = dtk::clamp(value, range.min(), range.max());
            _p->value->setIfChanged(tmp);
        }

        std::shared_ptr<dtk::IObservableValue<int> > IntModel::observeValue() const
        {
            return _p->value;
        }

        const dtk::RangeI& IntModel::getRange() const
        {
            return _p->range->get();
        }

        void IntModel::setRange(const dtk::RangeI& range)
        {
            DTK_P();
            if (p.range->setIfChanged(range))
            {
                setValue(p.value->get());
            }
        }

        std::shared_ptr<dtk::IObservableValue<dtk::RangeI> > IntModel::observeRange() const
        {
            return _p->range;
        }

        int IntModel::getStep() const
        {
            return _p->step;
        }

        void IntModel::setStep(int value)
        {
            _p->step = value;
        }

        void IntModel::incrementStep()
        {
            DTK_P();
            setValue(p.value->get() + p.step);
        }

        void IntModel::decrementStep()
        {
            DTK_P();
            setValue(p.value->get() - p.step);
        }

        int IntModel::getLargeStep() const
        {
            return _p->largeStep;
        }

        void IntModel::setLargeStep(int value)
        {
            _p->largeStep = value;
        }

        void IntModel::incrementLargeStep()
        {
            DTK_P();
            setValue(p.value->get() + p.largeStep);
        }

        void IntModel::decrementLargeStep()
        {
            DTK_P();
            setValue(p.value->get() - p.largeStep);
        }

        bool IntModel::hasDefaultValue() const
        {
            return _p->hasDefaultValue->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > IntModel::observeHasDefaultValue() const
        {
            return _p->hasDefaultValue;
        }

        int IntModel::getDefaultValue() const
        {
            return _p->defaultValue;
        }

        void IntModel::setDefaultValue(int value)
        {
            _p->hasDefaultValue->setIfChanged(true);
            _p->defaultValue = value;
        }

        void IntModel::setDefaultValue()
        {
            setValue(_p->defaultValue);
        }

        void IntModel::clearDefaultValue()
        {
            _p->hasDefaultValue->setIfChanged(false);
        }
    }
}
