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
            std::shared_ptr<imaging::FontSystem> fontSystem;
            imaging::Size frameBufferSize;
            float contentScale = 1.F;
            std::list<std::weak_ptr<IWidget> > topLevelWidgets;
            math::Vector2i cursorPos;
            std::weak_ptr<IWidget> hover;
            std::weak_ptr<IWidget> mousePress;
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

        void EventLoop::addWidget(const std::weak_ptr<IWidget>& widget)
        {
            TLRENDER_P();
            p.topLevelWidgets.push_back(widget);
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

        void EventLoop::tick()
        {
            TLRENDER_P();

            _tickEvent();

            if (_getSizeUpdate())
            {
                _sizeEvent();
                for (const auto& i : p.topLevelWidgets)
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
            for (const auto& i : p.topLevelWidgets)
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
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    out |= _getSizeUpdate(widget);
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
            for (const auto& i : p.topLevelWidgets)
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
            for (const auto& i : p.topLevelWidgets)
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
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    _drawEvent(widget, event);
                }
            }
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
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    out = _underCursor(widget, pos);
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
