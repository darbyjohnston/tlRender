// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollBar.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ScrollBar::Private
        {
            Orientation orientation = Orientation::Horizontal;
            int scrollSize = 0;
            int scrollPos = 0;
            std::function<void(int)> scrollPosCallback;

            struct SizeData
            {
                int border = 0;
                int handle = 0;
            };
            SizeData size;

            struct MouseData
            {
                bool inside = false;
                math::Vector2i pos;
                bool pressed = false;
                math::Vector2i pressedPos;
                int pressedScrollPos = 0;
            };
            MouseData mouse;
        };

        void ScrollBar::_init(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollBar", context, parent);
            TLRENDER_P();
            setMouseHover(true);
            p.orientation = orientation;
        }

        ScrollBar::ScrollBar() :
            _p(new Private)
        {}

        ScrollBar::~ScrollBar()
        {}

        std::shared_ptr<ScrollBar> ScrollBar::create(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ScrollBar>(new ScrollBar);
            out->_init(orientation, context, parent);
            return out;
        }

        void ScrollBar::setScrollSize(int value)
        {
            TLRENDER_P();
            if (value == p.scrollSize)
                return;
            p.scrollSize = value;
            _updates |= Update::Draw;
        }

        int ScrollBar::getScrollPos() const
        {
            return _p->scrollPos;
        }

        void ScrollBar::setScrollPos(int value)
        {
            TLRENDER_P();
            if (value == p.scrollPos)
                return;
            p.scrollPos = value;
            _updates |= Update::Draw;
        }

        void ScrollBar::setScrollPosCallback(const std::function<void(int)>& value)
        {
            _p->scrollPosCallback = value;
        }

        void ScrollBar::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void ScrollBar::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void ScrollBar::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            const int h = event.style->getSizeRole(SizeRole::Handle, event.displayScale);
            const int sa = event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);

            switch (p.orientation)
            {
            case Orientation::Horizontal:
                _sizeHint.x = sa;
                _sizeHint.y = h;
                break;
            case Orientation::Vertical:
                _sizeHint.x = h;
                _sizeHint.y = sa;
                break;
            default: break;
            }
            _sizeHint.x += p.size.border * 2;
            _sizeHint.y += p.size.border * 2;
        }

        void ScrollBar::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

            event.render->drawMesh(
                border(g, p.size.border),
                math::Vector2i(),
                event.style->getColorRole(ColorRole::Border));

            const math::BBox2i g2 = g.margin(-p.size.border);
            event.render->drawRect(
                g2,
                event.style->getColorRole(ColorRole::Base));

            const int scrollPosMax = _getScrollPosMax();
            if (scrollPosMax > 0)
            {
                const math::BBox2i g3 = _getHandleGeometry();
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Button));

                if (p.mouse.pressed)
                {
                    event.render->drawRect(
                        g3,
                        event.style->getColorRole(ColorRole::Pressed));
                }
                else if (p.mouse.inside)
                {
                    event.render->drawRect(
                        g3,
                        event.style->getColorRole(ColorRole::Hover));
                }
            }
        }

        void ScrollBar::mouseEnterEvent()
        {
            TLRENDER_P();
            p.mouse.inside = true;
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseLeaveEvent()
        {
            TLRENDER_P();
            p.mouse.inside = false;
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pos = event.pos;
            if (p.mouse.pressed)
            {
                int scrollPos = 0;
                const float s = _getScrollScale();
                switch (p.orientation)
                {
                case Orientation::Horizontal:
                    scrollPos = p.mouse.pressedScrollPos +
                        (event.pos.x - p.mouse.pressedPos.x) * s;
                    break;
                case Orientation::Vertical:
                    scrollPos = p.mouse.pressedScrollPos +
                        (event.pos.y - p.mouse.pressedPos.y) * s;
                    break;
                default: break;
                }
                const int scrollPosMax = _getScrollPosMax();
                const int scrollPosClamped = math::clamp(scrollPos, 0, scrollPosMax);
                if (scrollPosClamped != p.scrollPos)
                {
                    p.scrollPos = scrollPosClamped;
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                    if (p.scrollPosCallback)
                    {
                        p.scrollPosCallback(p.scrollPos);
                    }
                }
            }
        }

        void ScrollBar::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pos = event.pos;
            p.mouse.pressed = true;
            p.mouse.pressedPos = event.pos;
            p.mouse.pressedScrollPos = p.scrollPos;
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
            _updates |= Update::Draw;
        }

        int ScrollBar::_getScrollPosMax() const
        {
            TLRENDER_P();
            int out = 0;
            const math::BBox2i g = _geometry.margin(-p.size.border);
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                out = std::max(0, p.scrollSize - g.w() + 2);
                break;
            case Orientation::Vertical:
                out = std::max(0, p.scrollSize - g.h() + 2);
                break;
            default: break;
            }
            return out;
        }

        float ScrollBar::_getScrollScale() const
        {
            TLRENDER_P();
            float out = 0.F;
            const math::BBox2i& g = _geometry.margin(-p.size.border);
            switch (p.orientation)
            {
            case Orientation::Horizontal:
                out = g.w() > 0 ? (p.scrollSize / static_cast<float>(g.w())) : 0.F;
                break;
            case Orientation::Vertical:
                out = g.h() > 0 ? (p.scrollSize / static_cast<float>(g.h())) : 0.F;
                break;
            default: break;
            }
            return out;
        }

        math::BBox2i ScrollBar::_getHandleGeometry() const
        {
            TLRENDER_P();
            math::BBox2i out;
            const math::BBox2i& g = _geometry.margin(-p.size.border);
            switch (p.orientation)
            {
            case Orientation::Horizontal:
            {
                const int w = p.scrollSize - g.w();
                const int w2 = g.w() / static_cast<float>(p.scrollSize) * g.w();
                const int x = p.scrollPos / static_cast<float>(w) * (g.w() - w2);
                out = math::BBox2i(
                    g.x() + x,
                    g.y(),
                    w2,
                    g.h());
                break;
            }
            case Orientation::Vertical:
            {
                const int h = p.scrollSize - g.h();
                const int h2 = g.h() / static_cast<float>(p.scrollSize) * g.h();
                const int y = p.scrollPos / static_cast<float>(h) * (g.h() - h2);
                out = math::BBox2i(
                    g.x(),
                    g.y() + y,
                    g.w(),
                    h2);
                break;
            }
            default: break;
            }
            return out;
        }

        void ScrollBar::_resetMouse()
        {
            TLRENDER_P();
            if (p.mouse.pressed || p.mouse.inside)
            {
                p.mouse.pressed = false;
                p.mouse.inside = false;
                _updates |= Update::Draw;
            }
        }
    }
}
