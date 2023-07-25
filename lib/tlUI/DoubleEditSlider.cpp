// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/DoubleEditSlider.h>

#include <tlUI/DoubleEdit.h>
#include <tlUI/DoubleSlider.h>
#include <tlUI/IncButtons.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct DoubleEditSlider::Private
        {
            std::shared_ptr<DoubleModel> model;
            std::shared_ptr<DoubleEdit> edit;
            std::shared_ptr<DoubleSlider> slider;
            std::shared_ptr<HorizontalLayout> layout;
        };

        void DoubleEditSlider::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::DoubleEditSlider", context, parent);
            TLRENDER_P();

            setHStretch(Stretch::Expanding);

            p.model = model;
            if (!p.model)
            {
                p.model = DoubleModel::create(context);
            }

            p.edit = DoubleEdit::create(context, p.model);

            p.slider = DoubleSlider::create(context, p.model);

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.edit->setParent(p.layout);
            p.slider->setParent(p.layout);
            p.slider->setHStretch(ui::Stretch::Expanding);
        }

        DoubleEditSlider::DoubleEditSlider() :
            _p(new Private)
        {}

        DoubleEditSlider::~DoubleEditSlider()
        {}

        std::shared_ptr<DoubleEditSlider> DoubleEditSlider::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DoubleEditSlider>(new DoubleEditSlider);
            out->_init(context, model, parent);
            return out;
        }

        const std::shared_ptr<DoubleModel>& DoubleEditSlider::getModel() const
        {
            return _p->model;
        }

        void DoubleEditSlider::setDigits(int value)
        {
            _p->edit->setDigits(value);
        }

        void DoubleEditSlider::setPrecision(int value)
        {
            _p->edit->setPrecision(value);
        }

        void DoubleEditSlider::setFontRole(FontRole value)
        {
            _p->edit->setFontRole(value);
        }

        void DoubleEditSlider::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void DoubleEditSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
