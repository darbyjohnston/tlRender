// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/EventLoop.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/IPopup.h>
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
            math::Vector2i cursorPosPrev;
            std::weak_ptr<IWidget> hover;
            std::weak_ptr<IWidget> mousePress;
            MouseClickEvent mouseClickEvent;
            std::weak_ptr<IWidget> keyFocus;
            std::weak_ptr<IWidget> keyPress;
            KeyEvent keyEvent;
            std::shared_ptr<DragAndDropData> dragAndDropData;
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

            //if (auto focusWidget = p.keyFocus.lock())
            //{
            //    p.keyFocus.reset();
            //    focusWidget->keyFocusEvent(false);
            //}

            widget->setEventLoop(shared_from_this());
            p.topLevelWidgets.push_back(widget);

            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        void EventLoop::removeWidget(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();

            if (auto hover = p.hover.lock())
            {
                if (hover->getTopLevel() == widget)
                {
                    p.hover.reset();
                    hover->mouseLeaveEvent();
                    if (p.dragAndDropData)
                    {
                        hover->dragLeaveEvent(DragAndDropEvent(
                            p.cursorPos,
                            p.cursorPosPrev,
                            p.dragAndDropData));
                    }
                }
            }
            if (auto pressed = p.mousePress.lock())
            {
                if (pressed->getTopLevel() == widget)
                {
                    p.mousePress.reset();
                    p.mouseClickEvent.pos = p.cursorPos;
                    p.mouseClickEvent.accept = false;
                    pressed->mouseReleaseEvent(p.mouseClickEvent);
                }
            }
            if (auto focus = p.keyFocus.lock())
            {
                if (focus->getTopLevel() == widget)
                {
                    p.keyFocus.reset();
                    focus->keyFocusEvent(false);
                }
            }
            if (auto keyPress = p.keyPress.lock())
            {
                if (keyPress->getTopLevel() == widget)
                {
                    p.keyPress.reset();
                    p.keyEvent.pos = p.cursorPos;
                    p.keyEvent.accept = false;
                    keyPress->keyReleaseEvent(p.keyEvent);
                }
            }

            widget->setEventLoop(nullptr);
            auto i = std::find_if(
                p.topLevelWidgets.begin(),
                p.topLevelWidgets.end(),
                [widget](const std::weak_ptr<IWidget>& other)
                {
                    return widget == other.lock();
                });
            if (i != p.topLevelWidgets.end())
            {
                p.topLevelWidgets.erase(i);
            }

            p.updates |= Update::Size;
            p.updates |= Update::Draw;
        }

        bool EventLoop::key(Key key, bool press, int modifiers)
        {
            TLRENDER_P();
            p.keyEvent.key = key;
            p.keyEvent.modifiers = modifiers;
            p.keyEvent.pos = p.cursorPos;
            p.keyEvent.accept = false;
            if (press)
            {
                // First check if any popups should be closed.
                bool popupClose = false;
                if (Key::Escape == key && !p.topLevelWidgets.empty())
                {
                    if (auto popup = std::dynamic_pointer_cast<IPopup>(
                        p.topLevelWidgets.back().lock()))
                    {
                        popupClose = true;
                        popup->close();
                    }
                }

                if (!popupClose)
                {
                    // Send event to the focused widget.
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

                    // Send event to the hovered widget.
                    if (!p.keyEvent.accept)
                    {
                        auto widgets = _getUnderCursor(p.cursorPos);
                        for (auto i = widgets.rbegin(); i != widgets.rend(); ++i)
                        {
                            (*i)->keyPressEvent(p.keyEvent);
                            if (p.keyEvent.accept)
                            {
                                p.keyPress = *i;
                                break;
                            }
                        }
                    }

                    // Handle tab key navigation.
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
            }
            else if (auto widget = p.keyPress.lock())
            {
                widget->keyReleaseEvent(p.keyEvent);
            }
            return p.keyEvent.accept;
        }

        void EventLoop::text(const std::string& value)
        {
            TLRENDER_P();
            TextEvent event(value);

            // Send event to the focused widget.
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

            // Send event to the hovered widget.
            if (!event.accept)
            {
                auto widgets = _getUnderCursor(p.cursorPos);
                for (auto i = widgets.rbegin(); i != widgets.rend(); ++i)
                {
                    (*i)->textEvent(event);
                    if (event.accept)
                    {
                        break;
                    }
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

            p.cursorPosPrev = p.cursorPos;
            p.cursorPos = pos;

            MouseMoveEvent event(p.cursorPos, p.cursorPosPrev);
            if (auto widget = p.mousePress.lock())
            {
                widget->mouseMoveEvent(event);
                if (p.dragAndDropData)
                {
                    widget->dragMoveEvent(DragAndDropEvent(
                        p.cursorPos,
                        p.cursorPosPrev,
                        p.dragAndDropData));
                }
            }
            else
            {
                _hoverUpdate(event);
            }
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
                auto widgets = _getUnderCursor(p.cursorPos);
                auto i = widgets.rbegin();
                for (; i != widgets.rend(); ++i)
                {
                    (*i)->mousePressEvent(p.mouseClickEvent);
                    if (p.mouseClickEvent.accept)
                    {
                        p.mousePress = *i;
                        break;
                    }
                }

                // Close popups.
                auto j = widgets.rbegin();
                for (; j != i && j != widgets.rend(); ++j)
                {
                    if (auto popup = std::dynamic_pointer_cast<IPopup>(*j))
                    {
                        popup->close();
                    }
                }
            }
            else
            {
                if (auto widget = p.mousePress.lock())
                {
                    p.mousePress.reset();
                    widget->mouseReleaseEvent(p.mouseClickEvent);
                }

                _hoverUpdate(MouseMoveEvent(
                    p.cursorPos,
                    p.cursorPosPrev));
            }
        }

        void EventLoop::scroll(float dx, float dy, int modifiers)
        {
            TLRENDER_P();
            ScrollEvent event(modifiers, p.cursorPos, dx, dy);
            auto widgets = _getUnderCursor(p.cursorPos);
            for (auto i = widgets.rbegin(); i != widgets.rend(); ++i)
            {
                (*i)->scrollEvent(event);
                if (event.accept)
                {
                    break;
                }
            }
        }

        const std::shared_ptr<IClipboard>& EventLoop::getClipboard() const
        {
            return _p->clipboard;
        }

        void EventLoop::startDragAndDrop(const std::shared_ptr<DragAndDropData>& value)
        {

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

        void EventLoop::_tickEvent()
        {
            TLRENDER_P();
            p.widgetCount = 0;
            TickEvent event(
                p.style,
                p.iconLibrary,
                p.fontSystem,
                p.displayScale);
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
            SizeHintEvent event(
                p.style,
                p.iconLibrary,
                p.fontSystem,
                p.displayScale);
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
            ClipEvent event(
                p.style,
                p.iconLibrary,
                p.fontSystem,
                p.displayScale);
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
            DrawEvent event(
                p.style,
                p.iconLibrary,
                render,
                p.fontSystem,
                p.displayScale);
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

        std::list<std::shared_ptr<IWidget> > EventLoop::_getUnderCursor(
            const math::Vector2i& pos)
        {
            TLRENDER_P();
            std::list<std::shared_ptr<IWidget> > out;
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    if (!widget->isClipped() &&
                        widget->isEnabled() &&
                        widget->getGeometry().contains(pos))
                    {
                        out.push_back(widget);
                        _getUnderCursor(widget, pos, out);
                    }
                }
            }
            return out;
        }

        void EventLoop::_getUnderCursor(
            const std::shared_ptr<IWidget>& widget,
            const math::Vector2i& pos,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            for (const auto& child : widget->getChildren())
            {
                if (!child->isClipped() &&
                    child->isEnabled() &&
                    child->getGeometry().contains(pos))
                {
                    out.push_back(child);
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
                    widget->mouseLeaveEvent();                    if (p.dragAndDropData)
                    {
                        widget->dragLeaveEvent(DragAndDropEvent(
                            p.cursorPos,
                            p.cursorPosPrev,
                            p.dragAndDropData));
                    }

                    if (hover)
                    {
                        //std::cout << "enter: " << hover->getName() << std::endl;
                        hover->mouseEnterEvent();
                        if (p.dragAndDropData)
                        {
                            hover->dragEnterEvent(DragAndDropEvent(
                                p.cursorPos,
                                p.cursorPosPrev,
                                p.dragAndDropData));
                        }
                    }
                }
            }
            else if (hover)
            {
                //std::cout << "enter: " << hover->getName() << std::endl;
                hover->mouseEnterEvent();
                if (p.dragAndDropData)
                {
                    hover->dragEnterEvent(DragAndDropEvent(
                        p.cursorPos,
                        p.cursorPosPrev,
                        p.dragAndDropData));
                }
            }

            p.hover = hover;

            if (auto widget = p.hover.lock())
            {
                widget->mouseMoveEvent(MouseMoveEvent(
                    p.cursorPos,
                    p.cursorPosPrev));
                if (p.dragAndDropData)
                {
                    widget->dragMoveEvent(DragAndDropEvent(
                        p.cursorPos,
                        p.cursorPosPrev,
                        p.dragAndDropData));
                }
            }
        }

        void EventLoop::_hoverUpdate(MouseMoveEvent& event)
        {
            auto widgets = _getUnderCursor(event.pos);
            while (!widgets.empty())
            {
                if (widgets.back()->hasMouseHover())
                {
                    break;
                }
                widgets.pop_back();
            }
            _setHover(!widgets.empty() ? widgets.back() : nullptr);
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
