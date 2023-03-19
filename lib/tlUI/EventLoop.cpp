// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/EventLoop.h>

namespace tl
{
    namespace ui
    {
        struct EventLoop::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<Style> style;
            std::shared_ptr<IconLibrary> iconLibrary;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<imaging::FontSystem> fontSystem;
            imaging::Size frameBufferSize;
            float contentScale = 1.F;
            std::list<std::weak_ptr<Window> > windows;
            math::Vector2i cursorPos;
            std::weak_ptr<IWidget> hover;
            std::weak_ptr<IWidget> mousePress;
            std::weak_ptr<IWidget> keyPress;
        };

        void EventLoop::_init(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<timeline::IRender>& render,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
            p.style = style;
            p.iconLibrary = iconLibrary;
            p.render = render;
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
            const std::shared_ptr<timeline::IRender>& render,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<EventLoop>(new EventLoop);
            out->_init(style, iconLibrary, render, fontSystem, context);
            return out;
        }

        void EventLoop::setFrameBufferSize(const imaging::Size& value)
        {
            _p->frameBufferSize = value;
        }

        void EventLoop::setContentScale(float value)
        {
            _p->contentScale = value;
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
                auto hover = _underCursor(pos);
                //std::cout << "hover: " << (hover ? hover->getName() : "none") << std::endl;
                _setHover(hover);
                if (auto widget = p.hover.lock())
                {
                    widget->mouseMoveEvent(event);
                }
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
                auto hover = _underCursor(p.cursorPos);
                //std::cout << "hover: " << (hover ? hover->getName() : "none") << std::endl;
                _setHover(hover);
            }
        }

        void EventLoop::addWindow(const std::weak_ptr<Window>& window)
        {
            _p->windows.push_back(window);
        }

        void EventLoop::tick()
        {
            _tickEvent();
            _sizeEvent();
            _drawEvent();
        }

        void EventLoop::_tickEvent()
        {
            TLRENDER_P();
            TickEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.fontSystem = p.fontSystem;
            event.contentScale = p.contentScale;
            for (const auto& i : p.windows)
            {
                if (auto window = i.lock())
                {
                    _tickEvent(window, event);
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

        void EventLoop::_sizeEvent()
        {
            TLRENDER_P();
            SizeEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.fontSystem = p.fontSystem;
            event.contentScale = p.contentScale;
            for (const auto& i : p.windows)
            {
                if (auto window = i.lock())
                {
                    _sizeEvent(window, event);
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

        void EventLoop::_drawEvent()
        {
            TLRENDER_P();
            DrawEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.render = p.render;
            event.fontSystem = p.fontSystem;
            event.contentScale = p.contentScale;
            p.render->begin(p.frameBufferSize);
            for (const auto& i : p.windows)
            {
                if (auto window = i.lock())
                {
                    _drawEvent(window, event);
                }
            }
            p.render->end();
        }

        void EventLoop::_drawEvent(
            const std::shared_ptr<IWidget>& widget,
            const DrawEvent& event)
        {
            if (widget->isVisible())
            {
                widget->drawEvent(event);
                for (const auto& child : widget->getChildren())
                {
                    _drawEvent(child, event);
                }
            }
        }

        std::shared_ptr<IWidget> EventLoop::_underCursor(
            const math::Vector2i& pos)
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            for (const auto& i : p.windows)
            {
                if (auto window = i.lock())
                {
                    out = _underCursor(window, pos);
                }
            }
            return out;
        }

        std::shared_ptr<IWidget> EventLoop::_underCursor(
            const std::shared_ptr<IWidget>& widget,
            const math::Vector2i& pos)
        {
            std::shared_ptr<IWidget> out;
            if (widget->isVisible())
            {
                for (const auto& child : widget->getChildren())
                {
                    auto tmp = _underCursor(child, pos);
                    if (tmp)
                    {
                        out = tmp;
                        break;
                    }
                }
                if (!out && widget->getGeometry().contains(pos))
                {
                    out = widget;
                }
            }
            return out;
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
    }
}
