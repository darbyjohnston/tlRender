// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/EventLoop.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct EventLoop::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<Style> style;
            std::shared_ptr<IconLibrary> iconLibrary;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            imaging::Size frameBufferSize;
            float contentScale = 1.F;
            std::list<std::weak_ptr<IWidget> > widgets;
            math::Vector2i cursorPos;
            std::weak_ptr<IWidget> hover;
            std::weak_ptr<IWidget> mousePress;
            std::weak_ptr<IWidget> keyFocus;
            std::weak_ptr<IWidget> keyPress;
            int updates = 0;
        };

        void EventLoop::_init(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
            p.style = style;
            p.iconLibrary = iconLibrary;
            p.fontSystem = fontSystem;
        }

        EventLoop::EventLoop() :
            _p(new Private)
        {}

        EventLoop::~EventLoop()
        {}

        std::shared_ptr<EventLoop> EventLoop::create(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<EventLoop>(new EventLoop);
            out->_init(style, iconLibrary, fontSystem, context);
            return out;
        }

        void EventLoop::setSize(const imaging::Size& value)
        {
            TLRENDER_P();
            if (value == p.frameBufferSize )
                return;
            p.frameBufferSize = value;
            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        void EventLoop::setContentScale(float value)
        {
            TLRENDER_P();
            if (value == p.contentScale)
                return;
            p.contentScale = value;
            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        void EventLoop::addWidget(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();
            widget->setEventLoop(shared_from_this());
            p.widgets.push_back(widget);
            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        void EventLoop::key(Key key, bool press)
        {
            TLRENDER_P();
            KeyEvent event;
            event.key = key;
        }

        void EventLoop::cursorEnter(bool enter)
        {
            if (!enter)
            {
                _setHover(nullptr);
            }
        }

        void EventLoop::cursorPos(const math::Vector2i& pos)
        {
            TLRENDER_P();
            MouseMoveEvent event;
            event.pos = pos;
            event.prev = p.cursorPos;
            if (auto widget = p.mousePress.lock())
            {
                widget->mouseMoveEvent(event);
            }
            else
            {
                _hoverUpdate(event);
            }
            p.cursorPos = pos;
        }

        void EventLoop::mouseButton(int button, bool press, int modifiers)
        {
            TLRENDER_P();
            MouseClickEvent event;
            event.button = button;
            event.modifiers = modifiers;
            if (press)
            {
                if (auto widget = p.hover.lock())
                {
                    widget->mousePressEvent(event);
                    p.mousePress = widget;
                }
            }
            else
            {
                if (auto widget = p.mousePress.lock())
                {
                    widget->mouseReleaseEvent(event);
                    p.mousePress.reset();
                }

                MouseMoveEvent moveEvent;
                moveEvent.pos = p.cursorPos;
                moveEvent.prev = p.cursorPos;
                _hoverUpdate(moveEvent);
            }
        }

        void EventLoop::tick()
        {
            TLRENDER_P();

            _tickEvent();

            if (_getSizeUpdate())
            {
                _sizeEvent();
                for (const auto& i : p.widgets)
                {
                    if (auto widget = i.lock())
                    {
                        widget->setGeometry(math::BBox2i(
                            0,
                            0,
                            p.frameBufferSize.w,
                            p.frameBufferSize.h));
                    }
                }
                _p->updates &= ~static_cast<int>(Update::Size);
            }

            if (_getDrawUpdate())
            {
                p.updates |= Update::Draw;
            }
        }

        bool EventLoop::hasDrawUpdate() const
        {
            return _p->updates & Update::Draw;
        }

        void EventLoop::draw(const std::shared_ptr<timeline::IRender>& render)
        {
            _drawEvent(render);
            _p->updates &= ~static_cast<int>(Update::Draw);
        }

        void EventLoop::_tickEvent()
        {
            TLRENDER_P();
            TickEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.fontSystem = p.fontSystem;
            event.contentScale = p.contentScale;
            for (const auto& i : p.widgets)
            {
                if (auto widget = i.lock())
                {
                    _tickEvent(widget, event);
                }
            }
        }

        void EventLoop::_tickEvent(
            const std::shared_ptr<IWidget>& widget,
            const TickEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _tickEvent(child, event);
            }
            widget->tickEvent(event);
        }

        bool EventLoop::_getSizeUpdate()
        {
            TLRENDER_P();
            bool out = p.updates & Update::Size;
            for (const auto& i : p.widgets)
            {
                if (auto widget = i.lock())
                {
                    out |= _getSizeUpdate(widget);
                    if (out)
                    {
                        break;
                    }
                }
            }
            return out;
        }

        bool EventLoop::_getSizeUpdate(const std::shared_ptr<IWidget>& widget)
        {
            bool out = widget->getUpdates() & Update::Size;
            for (const auto& child : widget->getChildren())
            {
                out |= _getSizeUpdate(child);
            }
            return out;
        }

        void EventLoop::_sizeEvent()
        {
            TLRENDER_P();
            SizeEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.fontSystem = p.fontSystem;
            event.contentScale = p.contentScale;
            for (auto i : getFontRoleEnums())
            {
                event.fontInfo[i] = p.style->getFontRole(i);
                event.fontInfo[i].size *= p.contentScale;
                event.fontMetrics[i] = p.fontSystem->getMetrics(event.fontInfo[i]);
            }
            for (const auto& i : p.widgets)
            {
                if (auto widget = i.lock())
                {
                    _sizeEvent(widget, event);
                }
            }
        }

        void EventLoop::_sizeEvent(
            const std::shared_ptr<IWidget>& widget,
            const SizeEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeEvent(child, event);
            }
            widget->sizeEvent(event);
        }

        bool EventLoop::_getDrawUpdate()
        {
            TLRENDER_P();
            bool out = p.updates & Update::Draw;
            for (const auto& i : p.widgets)
            {
                if (auto widget = i.lock())
                {
                    out |= _getDrawUpdate(widget);
                    if (out)
                    {
                        break;
                    }
                }
            }
            return out;
        }

        bool EventLoop::_getDrawUpdate(const std::shared_ptr<IWidget>& widget)
        {
            bool out = false;
            if (widget->isVisible())
            {
                out = widget->getUpdates() & Update::Draw;
                for (const auto& child : widget->getChildren())
                {
                    out |= _getDrawUpdate(child);
                }
            }
            return out;
        }

        void EventLoop::_drawEvent(const std::shared_ptr<timeline::IRender>& render)
        {
            TLRENDER_P();
            DrawEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.render = render;
            event.fontSystem = p.fontSystem;
            event.contentScale = p.contentScale;
            for (auto i : getFontRoleEnums())
            {
                event.fontInfo[i] = p.style->getFontRole(i);
                event.fontInfo[i].size *= p.contentScale;
                event.fontMetrics[i] = p.fontSystem->getMetrics(event.fontInfo[i]);
            }
            event.render->setClipRectEnabled(true);
            for (const auto& i : p.widgets)
            {
                if (auto widget = i.lock())
                {
                    _drawEvent(
                        widget,
                        math::BBox2i(0, 0, p.frameBufferSize.w, p.frameBufferSize.h),
                        event);
                }
            }
            event.render->setClipRectEnabled(false);
        }

        void EventLoop::_drawEvent(
            const std::shared_ptr<IWidget>& widget,
            math::BBox2i clip,
            const DrawEvent& event)
        {
            if (widget->isVisible())
            {
                event.render->setClipRect(clip);
                widget->drawEvent(event);
                for (const auto& child : widget->getChildren())
                {
                    const math::BBox2i& g = child->getGeometry();
                    if (g.intersects(clip))
                    {
                        _drawEvent(child, g.intersect(clip), event);
                    }
                }
            }
        }

        void EventLoop::_underCursor(
            const math::Vector2i& pos,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            TLRENDER_P();
            for (const auto& i : p.widgets)
            {
                if (auto widget = i.lock())
                {
                    if (widget->isVisible() && widget->getGeometry().contains(pos))
                    {
                        _underCursor(widget, pos, out);
                        break;
                    }
                }
            }
        }

        void EventLoop::_underCursor(
            const std::shared_ptr<IWidget>& widget,
            const math::Vector2i& pos,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            out.push_back(widget);
            for (const auto& child : widget->getChildren())
            {
                if (child->isVisible() && child->getGeometry().contains(pos))
                {
                    _underCursor(child, pos, out);
                    break;
                }
            }
        }

        void EventLoop::_setHover(const std::shared_ptr<IWidget>& hover)
        {
            TLRENDER_P();
            if (auto widget = p.hover.lock())
            {
                if (hover != widget)
                {
                    //std::cout << "leave: " << widget->getName() << std::endl;
                    widget->leaveEvent();
                    if (hover)
                    {
                        //std::cout << "enter: " << hover->getName() << std::endl;
                        hover->enterEvent();
                    }
                }
            }
            else if (hover)
            {
                hover->enterEvent();
            }
            p.hover = hover;
        }

        void EventLoop::_hoverUpdate(MouseMoveEvent& event)
        {
            std::list<std::shared_ptr<IWidget> > hover;
            _underCursor(event.pos, hover);
            while (!hover.empty())
            {
                hover.back()->mouseMoveEvent(event);
                if (event.accept)
                {
                    _setHover(hover.back());
                    break;
                }
                hover.pop_back();
            }
            if (hover.empty())
            {
                _setHover(nullptr);
            }
        }
    }
}
