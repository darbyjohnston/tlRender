// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollArea.h>

namespace tl
{
    namespace ui
    {
        struct ScrollArea::Private
        {
            ScrollType scrollType = ScrollType::Both;
            std::shared_ptr<observer::Value<math::Vector2i> > scrollSize;
            std::shared_ptr<observer::Value<math::Vector2i> > scrollPos;
        };

        void ScrollArea::_init(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollArea", context, parent);
            TLRENDER_P();
            p.scrollType = scrollType;
            p.scrollSize = observer::Value<math::Vector2i>::create();
            p.scrollPos = observer::Value<math::Vector2i>::create();
            //setBackgroundRole(ColorRole::Base);
        }

        ScrollArea::ScrollArea() :
            _p(new Private)
        {}

        ScrollArea::~ScrollArea()
        {}

        std::shared_ptr<ScrollArea> ScrollArea::create(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ScrollArea>(new ScrollArea);
            out->_init(context, scrollType, parent);
            return out;
        }

        const math::Vector2i& ScrollArea::getScrollSize() const
        {
            return _p->scrollSize->get();
        }

        std::shared_ptr<observer::IValue<math::Vector2i> > ScrollArea::observeScrollSize() const
        {
            return _p->scrollSize;
        }

        const math::Vector2i& ScrollArea::getScrollPos() const
        {
            return _p->scrollPos->get();
        }

        std::shared_ptr<observer::IValue<math::Vector2i> > ScrollArea::observeScrollPos() const
        {
            return _p->scrollPos;
        }

        void ScrollArea::setScrollPos(const math::Vector2i& value)
        {
            TLRENDER_P();
            if (p.scrollPos->setIfChanged(value))
            {
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }
        }

        void ScrollArea::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::BBox2i g = value;
            const math::Vector2i& scrollPos = p.scrollPos->get();
            math::Vector2i scrollSize;
            for (const auto& child : _children)
            {
                const math::Vector2i sizeHint = child->getSizeHint();
                scrollSize.x = std::max(scrollSize.x, sizeHint.x);
                scrollSize.y = std::max(scrollSize.y, sizeHint.y);
                const math::BBox2i g2(
                    g.min.x - scrollPos.x,
                    g.min.y - scrollPos.y,
                    sizeHint.x,
                    sizeHint.y);
                child->setGeometry(g2);
            }
            p.scrollSize->setIfChanged(scrollSize);
        }

        void ScrollArea::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            _sizeHint = math::Vector2i();
            switch (p.scrollType)
            {
                case ScrollType::Horizontal:
                    _sizeHint.x =
                        event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);
                    for (const auto& child : _children)
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
                    }
                    break;
                case ScrollType::Vertical:
                    _sizeHint.y =
                        event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);
                    for (const auto& child : _children)
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                    }
                    break;
                case ScrollType::Both:
                    _sizeHint.x = _sizeHint.y =
                        event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);
                    break;
            }
        }
    }
}
