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
            imaging::Size displaySize;
            float displayScale = 1.F;
            std::list<std::weak_ptr<IWidget> > topLevelWidgets;
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

        void EventLoop::setDisplaySize(const imaging::Size& value)
        {
            TLRENDER_P();
            if (value == p.displaySize )
                return;
            p.displaySize = value;
            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        void EventLoop::setDisplayScale(float value)
        {
            TLRENDER_P();
            if (value == p.displayScale)
                return;
            p.displayScale = value;
            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        const std::weak_ptr<IWidget>& EventLoop::getKeyFocus() const
        {
            return _p->keyFocus;
        }

        void EventLoop::setKeyFocus(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (value == p.keyFocus.lock())
                return;
            p.keyFocus = value;
            p.updates |= Update::Draw;
        }

        void EventLoop::addWidget(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();
            widget->setEventLoop(shared_from_this());
            p.topLevelWidgets.push_back(widget);
            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        void EventLoop::key(Key key, bool press, int modifiers)
        {
            TLRENDER_P();
            KeyEvent event;
            event.key = key;
            event.modifiers = modifiers;
            event.pos = p.cursorPos;
            if (press)
            {
                if (auto widget = p.keyFocus.lock())
                {
                    widget->keyPressEvent(event);
                    if (event.accept)
                    {
                        p.keyPress = widget;
                    }
                }
                if (!event.accept)
                {
                    std::list<std::shared_ptr<IWidget> > widgets;
                    _getUnderCursor(p.cursorPos, widgets);
                    while (!widgets.empty())
                    {
                        auto widget = widgets.back();
                        widgets.pop_back();
                        widget->keyPressEvent(event);
                        if (event.accept)
                        {
                            p.keyPress = widget;
                            break;
                        }
                    }
                }
                if (!event.accept && Key::Tab == key)
                {
                    auto keyFocus = p.keyFocus.lock();
                    if (modifiers == static_cast<int>(KeyModifier::Shift))
                    {
                        keyFocus = _keyFocusPrev(keyFocus);
                    }
                    else
                    {
                        keyFocus = _keyFocusNext(keyFocus);
                    }
                    setKeyFocus(keyFocus);
                }
            }
            else if (auto widget = p.keyPress.lock())
            {
                widget->keyReleaseEvent(event);
            }
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
            event.pos = p.cursorPos;
            if (press)
            {
                std::list<std::shared_ptr<IWidget> > widgets;
                _getUnderCursor(p.cursorPos, widgets);
                while (!widgets.empty())
                {
                    auto widget = widgets.back();
                    widgets.pop_back();
                    widget->mousePressEvent(event);
                    if (event.accept)
                    {
                        p.mousePress = widget;
                        break;
                    }
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
                _sizeHintEvent();
                for (const auto& i : p.topLevelWidgets)
                {
                    if (auto widget = i.lock())
                    {
                        widget->setGeometry(math::BBox2i(
                            0,
                            0,
                            p.displaySize.w,
                            p.displaySize.h));
                    }
                }
                _clipEvent();
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
            event.displayScale = p.displayScale;
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

        void EventLoop::_sizeHintEvent()
        {
            TLRENDER_P();
            SizeHintEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.fontSystem = p.fontSystem;
            event.displayScale = p.displayScale;
            for (auto i : getFontRoleEnums())
            {
                event.fontMetrics[i] = p.fontSystem->getMetrics(
                    p.style->getFontRole(i, p.displayScale));
            }
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    _sizeHintEvent(widget, event);
                }
            }
        }

        void EventLoop::_sizeHintEvent(
            const std::shared_ptr<IWidget>& widget,
            const SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEvent(child, event);
            }
            widget->sizeHintEvent(event);
        }

        void EventLoop::_clipEvent()
        {
            TLRENDER_P();
            ClipEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.fontSystem = p.fontSystem;
            event.displayScale = p.displayScale;
            for (auto i : getFontRoleEnums())
            {
                event.fontMetrics[i] = p.fontSystem->getMetrics(
                    p.style->getFontRole(i, p.displayScale));
            }
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    _clipEvent(
                        widget,
                        widget->getGeometry(),
                        !widget->isVisible(),
                        event);
                }
            }
        }

        void EventLoop::_clipEvent(
            const std::shared_ptr<IWidget>& widget,
            math::BBox2i clip,
            bool clipped,
            const ClipEvent& event)
        {
            const math::BBox2i& g = widget->getGeometry();
            clipped |= !g.intersects(clip);
            clipped |= !widget->isVisible();
            widget->clipEvent(clipped, event);
            for (const auto& child : widget->getChildren())
            {
                _clipEvent(child, g.intersect(clip), clipped, event);
            }
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
            if (!widget->isClipped())
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
            event.displayScale = p.displayScale;
            for (auto i : getFontRoleEnums())
            {
                event.fontMetrics[i] = p.fontSystem->getMetrics(
                    p.style->getFontRole(i, p.displayScale));
            }
            event.focusWidget = p.keyFocus.lock();
            event.render->setClipRectEnabled(true);
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    _drawEvent(
                        widget,
                        math::BBox2i(0, 0, p.displaySize.w, p.displaySize.h),
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
            if (!widget->isClipped())
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

        void EventLoop::_getUnderCursor(
            const math::Vector2i& pos,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            TLRENDER_P();
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    if (!widget->isClipped() && widget->getGeometry().contains(pos))
                    {
                        _getUnderCursor(widget, pos, out);
                        break;
                    }
                }
            }
        }

        void EventLoop::_getUnderCursor(
            const std::shared_ptr<IWidget>& widget,
            const math::Vector2i& pos,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            out.push_back(widget);
            for (const auto& child : widget->getChildren())
            {
                if (!child->isClipped() && child->getGeometry().contains(pos))
                {
                    _getUnderCursor(child, pos, out);
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
            std::shared_ptr<IWidget> widget;
            std::list<std::shared_ptr<IWidget> > widgets;
            _getUnderCursor(event.pos, widgets);
            while (!widgets.empty())
            {
                widget = widgets.back();
                widgets.pop_back();
                widget->mouseMoveEvent(event);
                if (event.accept)
                {
                    break;
                }
                widget = nullptr;
            }
            _setHover(widget);
        }

        std::shared_ptr<IWidget> EventLoop::_keyFocusNext(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            std::list<std::shared_ptr<IWidget> > widgets;
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    if (!widget->isClipped())
                    {
                        _getKeyFocus(widget, widgets);
                        break;
                    }
                }
            }
            if (!widgets.empty())
            {
                auto i = std::find(widgets.begin(), widgets.end(), value);
                if (i != widgets.end())
                {
                    ++i;
                    if (i != widgets.end())
                    {
                        out = *i;
                    }
                    else
                    {
                        out = widgets.front();
                    }
                }
                if (!out)
                {
                    out = widgets.front();
                }
            }
            return out;
        }

        std::shared_ptr<IWidget> EventLoop::_keyFocusPrev(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            std::list<std::shared_ptr<IWidget> > widgets;
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    if (!widget->isClipped())
                    {
                        _getKeyFocus(widget, widgets);
                        break;
                    }
                }
            }
            if (!widgets.empty())
            {
                auto i = std::find(widgets.rbegin(), widgets.rend(), value);
                if (i != widgets.rend())
                {
                    ++i;
                    if (i != widgets.rend())
                    {
                        out = *i;
                    }
                    else
                    {
                        out = widgets.back();
                    }
                }
                if (!out)
                {
                    out = widgets.back();
                }
            }
            return out;
        }

        void EventLoop::_getKeyFocus(
            const std::shared_ptr<IWidget>& widget,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            if (widget->acceptsKeyFocus())
            {
                out.push_back(widget);
            }
            for (const auto& child : widget->getChildren())
            {
                if (!child->isClipped())
                {
                    _getKeyFocus(child, out);
                }
            }
        }
    }
}
