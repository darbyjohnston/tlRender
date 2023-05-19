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

        void StackLayout::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            for (const auto& child : _children)
            {
                child->setGeometry(value);
            }
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
            for (const auto& child : _children)
            {
                const math::Vector2i& sizeHint = child->getSizeHint();
                _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
            }
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
