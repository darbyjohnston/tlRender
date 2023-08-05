// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/StackLayout.h>

namespace tl
{
    namespace ui
    {
        struct StackLayout::Private
        {
            int currentIndex = 0;
            SizeRole marginRole = SizeRole::None;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;
        };

        void StackLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::StackLayout", context, parent);
        }

        StackLayout::StackLayout() :
            _p(new Private)
        {}

        StackLayout::~StackLayout()
        {}

        std::shared_ptr<StackLayout> StackLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StackLayout>(new StackLayout);
            out->_init(context, parent);
            return out;
        }

        int StackLayout::getCurrentIndex() const
        {
            return _p->currentIndex;
        }

        void StackLayout::setCurrentIndex(int value)
        {
            TLRENDER_P();
            if (value == p.currentIndex)
                return;
            p.currentIndex = value;
            _widgetUpdate();
        }

        void StackLayout::setCurrentWidget(const std::shared_ptr<IWidget>& value)
        {
            int index = 0;
            for (auto i = _children.begin(); i != _children.end(); ++i, ++index)
            {
                if (value == *i)
                {
                    setCurrentIndex(index);
                    break;
                }
            }
        }

        void StackLayout::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void StackLayout::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::Box2i g = _geometry.margin(-p.size.margin);
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        math::Box2i StackLayout::getChildrenClipRect() const
        {
            return _geometry.margin(-_p->size.margin);
        }

        void StackLayout::childAddedEvent(const ChildEvent& event)
        {
            _widgetUpdate();
        }

        void StackLayout::childRemovedEvent(const ChildEvent& event)
        {
            _widgetUpdate();
        }

        void StackLayout::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(p.marginRole, event.displayScale);

            _sizeHint = math::Vector2i();
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
            }
            _sizeHint.x += p.size.margin * 2;
            _sizeHint.y += p.size.margin * 2;
        }

        std::shared_ptr<IWidget> StackLayout::_getCurrentWidget() const
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            int i = 0;
            for (const auto& child : _children)
            {
                if (i == p.currentIndex)
                {
                    out = child;
                    break;
                }
                ++i;
            }
            return out;
        }

        void StackLayout::_widgetUpdate()
        {
            const auto currentWidget = _getCurrentWidget();
            for (const auto& child : _children)
            {
                child->setVisible(child == currentWidget);
            }
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }
    }
}
