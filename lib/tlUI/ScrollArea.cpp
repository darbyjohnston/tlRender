// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollArea.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ScrollArea::Private
        {
            ScrollAreaType scrollAreaType = ScrollAreaType::Both;
            std::shared_ptr<observer::Value<math::Vector2i> > scrollSize;
            std::shared_ptr<observer::Value<math::Vector2i> > scrollPos;
            bool border = true;

            struct Size
            {
                int border = 0;
            };
            Size size;
        };

        void ScrollArea::_init(
            const std::shared_ptr<system::Context>& context,
            ScrollAreaType scrollAreaType,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollArea", context, parent);
            TLRENDER_P();
            p.scrollAreaType = scrollAreaType;
            p.scrollSize = observer::Value<math::Vector2i>::create();
            p.scrollPos = observer::Value<math::Vector2i>::create();
        }

        ScrollArea::ScrollArea() :
            _p(new Private)
        {}

        ScrollArea::~ScrollArea()
        {}

        std::shared_ptr<ScrollArea> ScrollArea::create(
            const std::shared_ptr<system::Context>& context,
            ScrollAreaType scrollAreaType,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ScrollArea>(new ScrollArea);
            out->_init(context, scrollAreaType, parent);
            return out;
        }

        std::shared_ptr<observer::IValue<math::Vector2i> > ScrollArea::observeScrollSize() const
        {
            return _p->scrollSize;
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

        std::shared_ptr<observer::IValue<math::Vector2i> > ScrollArea::observeScrollPos() const
        {
            return _p->scrollPos;
        }

        void ScrollArea::setBorder(bool value)
        {
            TLRENDER_P();
            if (value == p.border)
                return;
            p.border = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void ScrollArea::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::Vector2i& scrollPos = p.scrollPos->get();
            math::Vector2i scrollSize;
            for (const auto& child : _children)
            {
                const math::Vector2i sizeHint = child->getSizeHint();
                scrollSize.x = std::max(scrollSize.x, sizeHint.x);
                scrollSize.y = std::max(scrollSize.y, sizeHint.y);
                math::BBox2i g;
                g.min = value.min - scrollPos;
                g.max = value.min + sizeHint - scrollPos;
                child->setGeometry(g);
            }
            p.scrollSize->setIfChanged(scrollSize);
        }

        void ScrollArea::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.size.border = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            _sizeHint = math::Vector2i();
            switch (p.scrollAreaType)
            {
                case ScrollAreaType::Horizontal:
                    _sizeHint.x = event.style->getSizeRole(SizeRole::ScrollArea);
                    for (const auto& child : _children)
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
                    }
                    break;
                case ScrollAreaType::Vertical:
                    _sizeHint.y = event.style->getSizeRole(SizeRole::ScrollArea);
                    for (const auto& child : _children)
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                    }
                    break;
                case ScrollAreaType::Both:
                    _sizeHint.x = _sizeHint.y =
                        event.style->getSizeRole(SizeRole::ScrollArea);
                    break;
            }
            if (p.border)
            {
                _sizeHint.x += p.size.border * 2;
                _sizeHint.y += p.size.border * 2;
            }
        }

        void ScrollArea::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            math::BBox2i g = _geometry;

            if (p.border)
            {
                event.render->drawMesh(
                    border(g, p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
                g = g.margin(-p.size.border);
            }

            event.render->drawRect(
                g,
                event.style->getColorRole(ColorRole::Base));
        }
    }
}
