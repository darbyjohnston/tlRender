// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Bellows.h>

#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct Bellows::Private
        {
            std::shared_ptr<ui::ListButton> button;
            std::shared_ptr<ui::IWidget> widget;
            std::shared_ptr<ui::VerticalLayout> layout;
        };

        void Bellows::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Bellows", context, parent);
            TLRENDER_P();

            p.button = ui::ListButton::create(context);
            p.button->setCheckable(true);
            p.button->setButtonRole(ColorRole::Button);

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.button->setParent(p.layout);

            p.button->setCheckedCallback(
                [this](bool value)
                {
                    if (_p->widget)
                    {
                        _p->widget->setVisible(value);
                    }
                });
        }

        Bellows::Bellows() :
            _p(new Private)
        {}

        Bellows::~Bellows()
        {}

        std::shared_ptr<Bellows> Bellows::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Bellows>(new Bellows);
            out->_init(context, parent);
            return out;
        }

        void Bellows::setText(const std::string& value)
        {
            _p->button->setText(value);
        }

        void Bellows::setWidget(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (value == p.widget)
                return;
            if (p.widget)
            {
                p.widget->setParent(nullptr);
            }
            p.widget = value;
            if (p.widget)
            {
                p.widget->setParent(_p->layout);
                p.widget->setVisible(p.button->isChecked());
            }
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Bellows::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void Bellows::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
