// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/Bellows.h>

#include <tlUI/Divider.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace ui
    {
        struct Bellows::Private
        {
            std::shared_ptr<ListButton> button;
            std::shared_ptr<IWidget> widget;
            std::shared_ptr<VerticalLayout> layout;
        };

        void Bellows::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Bellows", context, parent);
            TLRENDER_P();

            p.button = ListButton::create(context);
            p.button->setCheckable(true);
            p.button->setIcon("BellowsClosed");
            p.button->setCheckedIcon("BellowsOpen");
            p.button->setButtonRole(ColorRole::Button);
            p.button->setCheckedRole(ColorRole::Button);

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);
            p.button->setParent(p.layout);
            Divider::create(Orientation::Horizontal, context, p.layout);

            p.button->setCheckedCallback(
                [this](bool value)
                {
                    setOpen(value);
                });
        }

        Bellows::Bellows() :
            _p(new Private)
        {}

        Bellows::~Bellows()
        {}

        std::shared_ptr<Bellows> Bellows::create(
            const std::shared_ptr<dtk::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<Bellows>(new Bellows);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<Bellows> Bellows::create(
            const std::string& text,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Bellows>(new Bellows);
            out->_init(context, parent);
            out->setText(text);
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

        bool Bellows::isOpen() const
        {
            return _p->button->isChecked();
        }

        void Bellows::setOpen(bool value)
        {
            TLRENDER_P();
            p.button->setChecked(value);
            if (p.widget)
            {
                p.widget->setVisible(value);
            }
        }

        void Bellows::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void Bellows::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    }
}
