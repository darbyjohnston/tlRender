// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/EventLoop.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/IWidget.h>

#include <tlCore/StringFormat.h>

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
            std::shared_ptr<IClipboard> clipboard;
            imaging::Size displaySize;
            float displayScale = 1.F;
            std::list<std::weak_ptr<IWidget> > topLevelWidgets;
            math::Vector2i cursorPos;
            std::weak_ptr<IWidget> hover;
            std::weak_ptr<IWidget> mousePress;
            MouseClickEvent mouseClickEvent;
            std::weak_ptr<IWidget> keyFocus;
            std::weak_ptr<IWidget> keyPress;
            KeyEvent keyEvent;
            int updates = 0;
            size_t widgetCount = 0;
            std::list<int> tickTimes;
            std::chrono::steady_clock::time_point logTimer;

            std::shared_ptr<observer::ValueObserver<bool> > styleChangedObserver;
        };

        void EventLoop::_init(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<imaging::FontSystem>& fontSystem,
            const std::shared_ptr<IClipboard>& clipboard,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
            p.style = style;
            p.iconLibrary = iconLibrary;
            p.fontSystem = fontSystem;
            p.clipboard = clipboard;
            p.logTimer = std::chrono::steady_clock::now();

            p.styleChangedObserver = observer::ValueObserver<bool>::create(
                p.style->observeChanged(),
                [this](bool)
                {
                    _p->updates |= Update::Size;
                    _p->updates |= Update::Draw;
                });
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
            const std::shared_ptr<IClipboard>& clipboard,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<EventLoop>(new EventLoop);
            out->_init(style, iconLibrary, fontSystem, clipboard, context);
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

        void EventLoop::setKeyFocus(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (value == p.keyFocus.lock())
                return;
            if (auto widget = p.keyFocus.lock())
            {
                widget->keyFocusEvent(false);
            }
            p.keyFocus = value;
            if (auto widget = p.keyFocus.lock())
            {
                widget->keyFocusEvent(true);
            }
            p.updates |= Update::Draw;
        }

        void EventLoop::addWidget(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();

            widget->setEventLoop(shared_from_this());
            p.topLevelWidgets.push_back(widget);

            if (auto widget = p.hover.lock())
            {
                widget->leaveEvent();
                p.hover.reset();
            }
            if (auto widget = p.mousePress.lock())
            {
                p.mouseClickEvent.pos = p.cursorPos;
                p.mouseClickEvent.accept = false;
                widget->mouseReleaseEvent(p.mouseClickEvent);
                p.mousePress.reset();
            }
            if (auto widget = p.keyFocus.lock())
            {
                widget->keyFocusEvent(false);
                p.keyFocus.reset();
            }
            if (auto widget = p.keyPress.lock())
            {
                p.keyEvent.pos = p.cursorPos;
                p.keyEvent.accept = false;
                widget->keyReleaseEvent(p.keyEvent);
                p.keyPress.reset();
            }

            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        void EventLoop::key(Key key, bool press, int modifiers)
        {
            TLRENDER_P();
            p.keyEvent.key = key;
            p.keyEvent.modifiers = modifiers;
            p.keyEvent.pos = p.cursorPos;
            p.keyEvent.accept = false;
            if (press)
            {
                if (auto widget = p.keyFocus.lock())
                {
                    while (widget)
                    {
                        widget->keyPressEvent(p.keyEvent);
                        if (p.keyEvent.accept)
                        {
                            p.keyPress = widget;
                            break;
                        }
                        widget = widget->getParent().lock();
                    }
                }
                if (!p.keyEvent.accept)
                {
                    auto widget = _getUnderCursor(p.cursorPos);
                    while (widget)
                    {
                        widget->keyPressEvent(p.keyEvent);
                        if (p.keyEvent.accept)
                        {
                            p.keyPress = widget;
                            break;
                        }
                        widget = widget->getParent().lock();
                    }
                }
                if (!p.keyEvent.accept && Key::Tab == key)
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
                widget->keyReleaseEvent(p.keyEvent);
            }
        }

        void EventLoop::text(const std::string& value)
        {
            TLRENDER_P();
            TextEvent event;
            event.text = value;
            if (auto widget = p.keyFocus.lock())
            {
                while (widget)
                {
                    widget->textEvent(event);
                    if (event.accept)
                    {
                        break;
                    }
                    widget = widget->getParent().lock();
                }
            }
            if (!event.accept)
            {
                auto widget = _getUnderCursor(p.cursorPos);
                while (widget)
                {
                    widget->textEvent(event);
                    if (event.accept)
                    {
                        break;
                    }
                    widget = widget->getParent().lock();
                }
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
            event.pos = pos;// * p.displayScale;
            event.prev = p.cursorPos;
            if (auto widget = p.mousePress.lock())
            {
                widget->mouseMoveEvent(event);
            }
            else
            {
                _hoverUpdate(event);
            }
            p.cursorPos = event.pos;
        }

        void EventLoop::mouseButton(int button, bool press, int modifiers)
        {
            TLRENDER_P();
            p.mouseClickEvent.button = button;
            p.mouseClickEvent.modifiers = modifiers;
            p.mouseClickEvent.pos = p.cursorPos;
            p.mouseClickEvent.accept = false;
            if (press)
            {
                auto widget = _getUnderCursor(p.cursorPos);
                while (widget)
                {
                    widget->mousePressEvent(p.mouseClickEvent);
                    if (p.mouseClickEvent.accept)
                    {
                        p.mousePress = widget;
                        break;
                    }
                    widget = widget->getParent().lock();
                }
            }
            else
            {
                if (auto widget = p.mousePress.lock())
                {
                    widget->mouseReleaseEvent(p.mouseClickEvent);
                    p.mousePress.reset();
                }

                MouseMoveEvent moveEvent;
                moveEvent.pos = p.cursorPos;
                moveEvent.prev = p.cursorPos;
                _hoverUpdate(moveEvent);
            }
        }

        void EventLoop::scroll(float dx, float dy)
        {
            TLRENDER_P();
            ScrollEvent event;
            event.pos = p.cursorPos;
            event.dx = dx;
            event.dy = dy;
            auto widget = _getUnderCursor(p.cursorPos);
            while (widget)
            {
                widget->scrollEvent(event);
                if (event.accept)
                {
                    break;
                }
                widget = widget->getParent().lock();
            }
        }

        void EventLoop::tick()
        {
            TLRENDER_P();

            const auto tickTime0 = std::chrono::steady_clock::now();

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
                p.updates &= ~static_cast<int>(Update::Size);
            }

            if (_getDrawUpdate())
            {
                p.updates |= Update::Draw;
            }

            _purgeTopLevelWidgets();

            const auto tickTime1 = std::chrono::steady_clock::now();
            const auto tickDiff = std::chrono::duration_cast<std::chrono::milliseconds>(tickTime1 - tickTime0);
            p.tickTimes.push_back(tickDiff.count());
            while (p.tickTimes.size() > 60)
            {
                p.tickTimes.pop_front();
            }

            // Logging.
            const auto logTime = std::chrono::steady_clock::now();
            const std::chrono::duration<float> logDiff = logTime - p.logTimer;
            if (logDiff.count() > 10.F)
            {
                p.logTimer = logTime;
                if (auto context = p.context.lock())
                {
                    int tickAverage = 0;
                    for (const auto tickTime : p.tickTimes)
                    {
                        tickAverage += tickTime;
                    }
                    if (!p.tickTimes.empty())
                    {
                        tickAverage /= p.tickTimes.size();
                    }

                    auto logSystem = context->getLogSystem();
                    logSystem->print(
                        string::Format("tl::ui::EventLoop {0}").arg(this),
                        string::Format(
                            "\n"
                            "    Tick average: {0}ms\n"
                            "    Widget count: {1}").
                        arg(tickAverage).
                        arg(p.widgetCount));
                }
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

        const std::shared_ptr<IClipboard>& EventLoop::getClipboard() const
        {
            return _p->clipboard;
        }

        void EventLoop::_tickEvent()
        {
            TLRENDER_P();
            p.widgetCount = 0;
            TickEvent event;
            event.style = p.style;
            event.iconLibrary = p.iconLibrary;
            event.fontSystem = p.fontSystem;
            event.displayScale = p.displayScale;
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    _tickEvent(
                        widget,
                        widget->isVisible(false),
                        widget->isEnabled(false),
                        event);
                }
            }
        }

        void EventLoop::_tickEvent(
            const std::shared_ptr<IWidget>& widget,
            bool visible,
            bool enabled,
            const TickEvent& event)
        {
            TLRENDER_P();
            const bool parentsVisible = visible && widget->isVisible(false);
            const bool parentsEnabled = enabled && widget->isEnabled(false);
            for (const auto& child : widget->getChildren())
            {
                _tickEvent(
                    child,
                    parentsVisible,
                    parentsEnabled,
                    event);
            }
            widget->tickEvent(visible, enabled, event);
            p.widgetCount = p.widgetCount + 1;
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
            if (out)
            {
                //std::cout << "Size update: " << widget->getName() << std::endl;
            }
            else
            {
                for (const auto& child : widget->getChildren())
                {
                    out |= _getSizeUpdate(child);
                }
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
                        !widget->isVisible(false),
                        event);
                }
            }
        }

        void EventLoop::_clipEvent(
            const std::shared_ptr<IWidget>& widget,
            const math::BBox2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            const math::BBox2i& g = widget->getGeometry();
            clipped |= !g.intersects(clipRect);
            clipped |= !widget->isVisible(false);
            const math::BBox2i clipRect2 = g.intersect(clipRect);
            widget->clipEvent(clipRect2, clipped, event);
            const math::BBox2i childrenClipRect =
                widget->getChildrenClipRect().intersect(clipRect2);
            for (const auto& child : widget->getChildren())
            {
                const math::BBox2i& childGeometry = child->getGeometry();
                _clipEvent(
                    child,
                    childGeometry.intersect(childrenClipRect),
                    clipped,
                    event);
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
                if (out)
                {
                    //std::cout << "Draw update: " << widget->getName() << std::endl;
                }
                else
                {
                    for (const auto& child : widget->getChildren())
                    {
                        out |= _getDrawUpdate(child);
                    }
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
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            if (!widget->isClipped() && widget->getGeometry().isValid())
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const math::BBox2i childrenClipRect =
                    widget->getChildrenClipRect().intersect(drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const math::BBox2i& childGeometry = child->getGeometry();
                    if (childGeometry.intersects(childrenClipRect))
                    {
                        _drawEvent(
                            child,
                            childGeometry.intersect(childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }

        std::shared_ptr<IWidget> EventLoop::_getUnderCursor(
            const math::Vector2i& pos)
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            for (auto i = p.topLevelWidgets.rbegin();
                i != p.topLevelWidgets.rend();
                ++i)
            {
                if (auto widget = i->lock())
                {
                    if (!widget->isClipped() &&
                        widget->isEnabled() &&
                        widget->getGeometry().contains(pos))
                    {
                        out = _getUnderCursor(widget, pos);
                        break;
                    }
                }
            }
            return out;
        }

        std::shared_ptr<IWidget> EventLoop::_getUnderCursor(
            const std::shared_ptr<IWidget>& widget,
            const math::Vector2i& pos)
        {
            std::shared_ptr<IWidget> out = widget;
            for (const auto& child : widget->getChildren())
            {
                if (!child->isClipped() &&
                    child->isEnabled() &&
                    child->getGeometry().contains(pos))
                {
                    out = _getUnderCursor(child, pos);
                    break;
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

        void EventLoop::_hoverUpdate(MouseMoveEvent& event)
        {
            auto widget = _getUnderCursor(event.pos);
            while (widget)
            {
                widget->mouseMoveEvent(event);
                if (event.accept)
                {
                    break;
                }
                widget = widget->getParent().lock();
            }
            _setHover(widget);
        }

        std::shared_ptr<IWidget> EventLoop::_keyFocusNext(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            std::shared_ptr<IWidget> out;
            std::list<std::shared_ptr<IWidget> > widgets;
            if (!p.topLevelWidgets.empty())
            {
                if (auto widget = p.topLevelWidgets.back().lock())
                {
                    if (!widget->isClipped() && widget->isEnabled())
                    {
                        _getKeyFocus(widget, widgets);
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
            if (!p.topLevelWidgets.empty())
            {
                if (auto widget = p.topLevelWidgets.back().lock())
                {
                    if (!widget->isClipped() && widget->isEnabled())
                    {
                        _getKeyFocus(widget, widgets);
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
                if (!child->isClipped() && child->isEnabled())
                {
                    _getKeyFocus(child, out);
                }
            }
        }

        void EventLoop::_purgeTopLevelWidgets()
        {
            TLRENDER_P();
            auto i = p.topLevelWidgets.begin();
            while (i != p.topLevelWidgets.end())
            {
                if (i->expired())
                {
                    i = p.topLevelWidgets.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }
}
