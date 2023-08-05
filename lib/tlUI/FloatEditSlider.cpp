// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FloatEditSlider.h>

#include <tlUI/FloatEdit.h>
#include <tlUI/FloatSlider.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace ui
    {
        struct FloatEditSlider::Private
        {
            std::shared_ptr<FloatModel> model;

            std::shared_ptr<FloatEdit> edit;
            std::shared_ptr<FloatSlider> slider;
            std::shared_ptr<ToolButton> resetButton;
            std::shared_ptr<HorizontalLayout> layout;

            std::function<void(float)> callback;

            std::shared_ptr<observer::ValueObserver<float> > valueObserver;
            std::shared_ptr<observer::ValueObserver<bool> > hasDefaultObserver;
        };

        void FloatEditSlider::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<FloatModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::FloatEditSlider", context, parent);
            TLRENDER_P();

            setHStretch(Stretch::Expanding);

            p.model = model;
            if (!p.model)
            {
                p.model = FloatModel::create(context);
            }

            p.edit = FloatEdit::create(context, p.model);

            p.slider = FloatSlider::create(context, p.model);

            p.resetButton = ToolButton::create(context);
            p.resetButton->setIcon("Reset");
            p.resetButton->setToolTip("Reset to the default value");

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.edit->setParent(p.layout);
            p.slider->setParent(p.layout);
            p.slider->setHStretch(ui::Stretch::Expanding);
            p.resetButton->setParent(p.layout);

            p.resetButton->setClickedCallback(
                [this]
                {
                    _p->model->setDefaultValue();
                });

            p.valueObserver = observer::ValueObserver<float>::create(
                p.model->observeValue(),
                [this](float value)
                {
                    _p->resetButton->setEnabled(value != _p->model->getDefaultValue());
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });

            p.hasDefaultObserver = observer::ValueObserver<bool>::create(
                p.model->observeHasDefaultValue(),
                [this](bool value)
                {
                    _p->resetButton->setVisible(value);
                });
        }

        FloatEditSlider::FloatEditSlider() :
            _p(new Private)
        {}

        FloatEditSlider::~FloatEditSlider()
        {}

        std::shared_ptr<FloatEditSlider> FloatEditSlider::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<FloatModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FloatEditSlider>(new FloatEditSlider);
            out->_init(context, model, parent);
            return out;
        }

        float FloatEditSlider::getValue() const
        {
            return _p->model->getValue();
        }

        void FloatEditSlider::setValue(float value)
        {
            _p->model->setValue(value);
        }

        void FloatEditSlider::setCallback(const std::function<void(float)>& value)
        {
            _p->callback = value;
        }

        const math::FloatRange& FloatEditSlider::getRange() const
        {
            return _p->model->getRange();
        }

        void FloatEditSlider::setRange(const math::FloatRange& value)
        {
            _p->model->setRange(value);
        }

        void FloatEditSlider::setStep(float value)
        {
            _p->model->setStep(value);
        }

        void FloatEditSlider::setLargeStep(float value)
        {
            _p->model->setLargeStep(value);
        }

        void FloatEditSlider::setDefaultValue(float value)
        {
            _p->model->setDefaultValue(value);
        }

        const std::shared_ptr<FloatModel>& FloatEditSlider::getModel() const
        {
            return _p->model;
        }

        void FloatEditSlider::setDigits(int value)
        {
            _p->edit->setDigits(value);
        }

        void FloatEditSlider::setPrecision(int value)
        {
            _p->edit->setPrecision(value);
        }

        void FloatEditSlider::setFontRole(FontRole value)
        {
            _p->edit->setFontRole(value);
        }

        void FloatEditSlider::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FloatEditSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
