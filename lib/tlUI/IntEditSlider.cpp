// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntEditSlider.h>

#include <tlUI/IntEdit.h>
#include <tlUI/IntSlider.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct IntEditSlider::Private
        {
            std::shared_ptr<IntModel> model;
            std::shared_ptr<IntEdit> edit;
            std::shared_ptr<IntSlider> slider;
            std::shared_ptr<HorizontalLayout> layout;
        };

        void IntEditSlider::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::IntEditSlider", context, parent);
            TLRENDER_P();

            setHStretch(Stretch::Expanding);

            p.model = model;
            if (!p.model)
            {
                p.model = IntModel::create(context);
            }

            p.edit = IntEdit::create(context, p.model);

            p.slider = IntSlider::create(context, p.model);

            p.layout = HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::SpacingTool);
            p.edit->setParent(p.layout);
            p.slider->setParent(p.layout);
            p.slider->setHStretch(Stretch::Expanding);
        }

        IntEditSlider::IntEditSlider() :
            _p(new Private)
        {}

        IntEditSlider::~IntEditSlider()
        {}

        std::shared_ptr<IntEditSlider> IntEditSlider::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IntModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<IntEditSlider>(new IntEditSlider);
            out->_init(context, model, parent);
            return out;
        }

        const std::shared_ptr<IntModel>& IntEditSlider::getModel() const
        {
            return _p->model;
        }

        void IntEditSlider::setDigits(int value)
        {
            _p->edit->setDigits(value);
        }

        void IntEditSlider::setFontRole(FontRole value)
        {
            _p->edit->setFontRole(value);
        }

        void IntEditSlider::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void IntEditSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
