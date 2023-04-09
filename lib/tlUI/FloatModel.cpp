// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FloatModel.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace ui
    {
        struct FloatModel::Private
        {
            std::shared_ptr<observer::Value<float> > value;
            std::shared_ptr<observer::Value<math::FloatRange> > range;
            float step = .1F;
            float largeStep = 1.F;
        };

        void FloatModel::_init(const std::shared_ptr<system::Context>&)
        {
            TLRENDER_P();
            p.value = observer::Value<float>::create(0.F);
            p.range = observer::Value<math::FloatRange>::create(math::FloatRange(0.F, 1.F));
        }

        FloatModel::FloatModel() :
            _p(new Private)
        {}

        FloatModel::~FloatModel()
        {}

        std::shared_ptr<FloatModel> FloatModel::create(
            const std::shared_ptr<system::Context>& context)
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
            TLRENDER_P();
            const math::FloatRange& range = p.range->get();
            const float tmp = math::clamp(value, range.getMin(), range.getMax());
            _p->value->setIfChanged(tmp);
        }

        std::shared_ptr<observer::IValue<float> > FloatModel::observeValue() const
        {
            return _p->value;
        }

        const math::FloatRange& FloatModel::getRange() const
        {
            return _p->range->get();
        }

        void FloatModel::setRange(const math::FloatRange& range)
        {
            TLRENDER_P();
            if (p.range->setIfChanged(range))
            {
                setValue(p.value->get());
            }
        }

        std::shared_ptr<observer::IValue<math::FloatRange> > FloatModel::observeRange() const
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

        void FloatModel::addStep()
        {
            TLRENDER_P();
            setValue(p.value->get() + p.step);
        }

        void FloatModel::subtractStep()
        {
            TLRENDER_P();
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

        void FloatModel::addLargeStep()
        {
            TLRENDER_P();
            setValue(p.value->get() + p.largeStep);
        }

        void FloatModel::substractLargeStep()
        {
            TLRENDER_P();
            setValue(p.value->get() - p.largeStep);
        }
    }
}
