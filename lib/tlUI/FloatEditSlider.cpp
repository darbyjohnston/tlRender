// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FloatEditSlider.h>

#include <tlUI/IncButtons.h>
#include <tlUI/FloatEdit.h>
#include <tlUI/FloatSlider.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct FloatEditSlider::Private
        {
            std::shared_ptr<FloatModel> model;
            std::shared_ptr<FloatEdit> edit;
            std::shared_ptr<FloatIncButtons> incButtons;
            std::shared_ptr<FloatSlider> slider;
            std::shared_ptr<HorizontalLayout> layout;
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

            p.incButtons = FloatIncButtons::create(p.model, context);

            p.slider = FloatSlider::create(context, p.model);

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.edit->setParent(p.layout);
            p.incButtons->setParent(p.layout);
            p.slider->setParent(p.layout);
            p.slider->setHStretch(ui::Stretch::Expanding);
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

        void FloatEditSlider::setGeometry(const math::BBox2i& value)
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
