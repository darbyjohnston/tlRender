// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollBar.h>

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

            struct MouseData
            {
                bool inside = false;
                bool pressed = false;
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
        }

        void ScrollBar::drawEvent(const DrawEvent& event)
        {
            IWidget::drawEvent(event);
            TLRENDER_P();

            const math::BBox2i g = _geometry;

            event.render->drawRect(
                g,
                event.style->getColorRole(ColorRole::Base));

            math::BBox2i g2;
            switch (p.orientation)
            {
            case Orientation::Horizontal:
            {
                const int w = p.scrollSize - g.w();
                const int w2 = g.w() / static_cast<float>(p.scrollSize) * g.w();
                const int x = p.scrollPos / static_cast<float>(w) * (g.w() - w2);
                g2 = math::BBox2i(
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
                g2 = math::BBox2i(
                    g.x(),
                    g.y() + y,
                    g.w(),
                    h2);
                break;
            }
            default: break;
            }
            event.render->drawRect(
                g2,
                event.style->getColorRole(ColorRole::Button));
        }

        void ScrollBar::enterEvent()
        {
            TLRENDER_P();
            p.mouse.inside = true;
            _updates |= Update::Draw;
        }

        void ScrollBar::leaveEvent()
        {
            TLRENDER_P();
            p.mouse.inside = false;
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            if (p.mouse.pressed)
            {
                event.accept = true;
                const int scrollPos = _getScrollPos(event.pos);
                if (scrollPos != p.scrollPos)
                {
                    p.scrollPos = scrollPos;
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
            p.mouse.pressed = true;
            const int scrollPos = _getScrollPos(event.pos);
            if (scrollPos != p.scrollPos)
            {
                p.scrollPos = scrollPos;
                _updates |= Update::Size;
                _updates |= Update::Draw;
                if (p.scrollPosCallback)
                {
                    p.scrollPosCallback(p.scrollPos);
                }
            }
            _updates |= Update::Draw;
        }

        void ScrollBar::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
            _updates |= Update::Draw;
        }

        int ScrollBar::_getScrollPos(const math::Vector2i& value) const
        {
            TLRENDER_P();
            int out = 0;
            const math::BBox2i g = _geometry;
            switch (p.orientation)
            {
            case Orientation::Horizontal:
            {
                const int w = p.scrollSize - g.w();
                const int w2 = g.w() / static_cast<float>(p.scrollSize) * g.w();
                out = math::clamp(
                    static_cast<int>((value.x - g.x()) / static_cast<float>(g.w() - w2) * w),
                    0,
                    w);
                break;
            }
            case Orientation::Vertical:
            {
                const int h = p.scrollSize - g.h();
                const int h2 = g.h() / static_cast<float>(p.scrollSize) * g.h();
                out = math::clamp(
                    static_cast<int>((value.y - g.y()) / static_cast<float>(g.h() - h2) * h),
                    0,
                    h);
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
