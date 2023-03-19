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
            bool border = true;
            int borderSize = 0;
        };

        void ScrollArea::_init(
            const std::shared_ptr<system::Context>& context,
            ScrollAreaType scrollAreaType,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollArea", context, parent);
            TLRENDER_P();
            p.scrollAreaType = scrollAreaType;
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

        void ScrollArea::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::BBox2i g = value.margin(-p.borderSize);
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        void ScrollArea::sizeEvent(const SizeEvent& event)
        {
            TLRENDER_P();

            p.borderSize = event.style->getSizeRole(SizeRole::Border) * event.contentScale;

            _sizeHint.x = 0;
            _sizeHint.y = 0;
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
                _sizeHint.x += p.borderSize * 2;
                _sizeHint.y += p.borderSize * 2;
            }
        }

        void ScrollArea::drawEvent(const DrawEvent& event)
        {
            TLRENDER_P();

            math::BBox2i g = _geometry;

            if (p.border)
            {
                event.render->drawMesh(
                    border(g, p.borderSize),
                    event.style->getColorRole(ColorRole::Border));
                g = g.margin(-p.borderSize);
            }

            event.render->drawRect(
                g,
                event.style->getColorRole(ColorRole::Base));
        }
    }
}
