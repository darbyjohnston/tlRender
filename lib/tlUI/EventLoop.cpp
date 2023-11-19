// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/EventLoop.h>

#include <tlUI/ToolTip.h>

#include <tlTimeline/IRender.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            std::chrono::milliseconds toolTipTimeout(1000);
            float toolTipDistance = 10.F;
        }

        struct EventLoop::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<Style> style;
            std::shared_ptr<IconLibrary> iconLibrary;
            std::shared_ptr<image::FontSystem> fontSystem;
            std::shared_ptr<IClipboard> clipboard;
            math::Size2i displaySize;
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
            std::shared_ptr<DragAndDropData> dndData;
            std::shared_ptr<image::Image> dndCursor;
            math::Vector2i dndCursorHotspot;
            std::weak_ptr<IWidget> dndHover;
            std::shared_ptr<ToolTip> toolTip;
            math::Vector2i toolTipPos;
            std::chrono::steady_clock::time_point toolTipTimer;
            int updates = 0;
            size_t widgetCount = 0;
            std::list<int> tickTimes;
            std::chrono::steady_clock::time_point logTimer;

            std::shared_ptr<observer::ValueObserver<bool> > styleChangedObserver;
        };

        void EventLoop::_init(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<IClipboard>& clipboard,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.context = context;
            p.style = style;
            p.iconLibrary = iconLibrary;
            p.fontSystem = context->getSystem<image::FontSystem>();
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
            const std::shared_ptr<IClipboard>& clipboard,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<EventLoop>(new EventLoop);
            out->_init(style, iconLibrary, clipboard, context);
            return out;
        }

        void EventLoop::setDisplaySize(const math::Size2i& value)
        {
            TLRENDER_P();
            if (value == p.displaySize)
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
            if (auto dragAndDrop = p.dndHover.lock())
            {
                p.dndHover.reset();
                DragAndDropEvent event(
                    p.cursorPos,
                    p.cursorPosPrev,
                    p.dndData);
                dragAndDrop->dragLeaveEvent(event);
            }
            p.dndData.reset();
            p.dndCursor.reset();
            _clipEvent(widget, widget->getGeometry(), true);

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
                    // Send event to the focused widget or parent.
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
                        for (auto i = widgets.begin(); i != widgets.end(); ++i)
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
                for (auto i = widgets.begin(); i != widgets.end(); ++i)
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
                if (p.dndData)
                {
                    // Find the drag and drop hover widget.
                    DragAndDropEvent event(
                        p.cursorPos,
                        p.cursorPosPrev,
                        p.dndData);
                    auto hover = p.dndHover.lock();
                    auto widgets = _getUnderCursor(p.cursorPos);
                    std::shared_ptr<IWidget> widget;
                    while (!widgets.empty())
                    {
                        if (hover == widgets.front())
                        {
                            break;
                        }
                        widgets.front()->dragEnterEvent(event);
                        if (event.accept)
                        {
                            widget = widgets.front();
                            break;
                        }
                        widgets.pop_front();
                    }
                    if (widget)
                    {
                        if (hover)
                        {
                            hover->dragLeaveEvent(event);
                        }
                        p.dndHover = widget;
                    }
                    else if (widgets.empty() && hover)
                    {
                        p.dndHover.reset();
                        hover->dragLeaveEvent(event);
                    }
                    hover = p.dndHover.lock();
                    if (hover)
                    {
                        DragAndDropEvent event(
                            p.cursorPos,
                            p.cursorPosPrev,
                            p.dndData);
                        hover->dragMoveEvent(event);
                    }
                }
                else
                {
                    widget->mouseMoveEvent(event);

                    p.dndData = event.dndData;
                    p.dndCursor = event.dndCursor;
                    p.dndCursorHotspot = event.dndCursorHotspot;
                    if (p.dndData)
                    {
                        // Start a drag and drop.
                        widget->mouseReleaseEvent(p.mouseClickEvent);
                        widget->mouseLeaveEvent();
                    }
                }
            }
            else
            {
                _hoverUpdate(event);
            }

            if (p.dndCursor)
            {
                p.updates |= Update::Draw;
            }

            if (math::length(p.cursorPos - p.toolTipPos) > toolTipDistance)
            {
                if (p.toolTip)
                {
                    p.toolTip->close();
                    p.toolTip.reset();
                }
                p.toolTipTimer = std::chrono::steady_clock::now();
                p.toolTipPos = p.cursorPos;
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
                auto i = widgets.begin();
                for (; i != widgets.end(); ++i)
                {
                    (*i)->mousePressEvent(p.mouseClickEvent);
                    if (p.mouseClickEvent.accept)
                    {
                        p.mousePress = *i;
                        break;
                    }
                }

                // Close popups.
                auto j = widgets.begin();
                for (; j != i && j != widgets.end(); ++j)
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
                    if (auto hover = p.dndHover.lock())
                    {
                        // Finish a drag and drop.
                        p.dndHover.reset();
                        DragAndDropEvent event(
                            p.cursorPos,
                            p.cursorPosPrev,
                            p.dndData);
                        hover->dragLeaveEvent(event);
                        hover->dropEvent(event);
                    }
                    else
                    {
                        widget->mouseReleaseEvent(p.mouseClickEvent);
                    }
                    p.dndData.reset();
                    p.dndCursor.reset();
                    p.updates |= Update::Draw;
                }

                MouseMoveEvent event(
                    p.cursorPos,
                    p.cursorPosPrev);
                _hoverUpdate(event);
            }
        }

        void EventLoop::scroll(const math::Vector2f& value, int modifiers)
        {
            TLRENDER_P();
            ScrollEvent event(value, modifiers, p.cursorPos);
            auto widgets = _getUnderCursor(p.cursorPos);
            for (auto i = widgets.begin(); i != widgets.end(); ++i)
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
                        widget->setGeometry(math::Box2i(
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

            const auto toolTipTime = std::chrono::steady_clock::now();
            const auto toolTipDiff = std::chrono::duration_cast<std::chrono::milliseconds>(toolTipTime - p.toolTipTimer);
            if (toolTipDiff > toolTipTimeout && !p.toolTip)
            {
                if (auto context = p.context.lock())
                {
                    std::string text;
                    auto widgets = _getUnderCursor(p.cursorPos);
                    while (!widgets.empty())
                    {
                        text = widgets.front()->getToolTip();
                        if (!text.empty())
                        {
                            break;
                        }
                        widgets.pop_front();
                    }
                    if (!text.empty())
                    {
                        p.toolTip = ToolTip::create(
                            text,
                            p.cursorPos,
                            shared_from_this(),
                            context);
                        p.toolTipPos = p.cursorPos;
                    }
                }
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
            TLRENDER_P();
            _drawEvent(render);
            if (p.dndCursor)
            {
                render->drawImage(
                    p.dndCursor,
                    math::Box2i(
                        p.cursorPos.x - p.dndCursorHotspot.x,
                        p.cursorPos.y - p.dndCursorHotspot.y,
                        p.dndCursor->getWidth(),
                        p.dndCursor->getHeight()),
                    image::Color4f(1.F, 1.F, 1.F));
            }
            _p->updates &= ~static_cast<int>(Update::Draw);
        }

        void EventLoop::_tickEvent()
        {
            TLRENDER_P();
            p.widgetCount = 0;
            TickEvent event(
                p.style,
                p.iconLibrary,
                p.fontSystem);
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
                //std::cout << "Size update: " << widget->getObjectName() << std::endl;
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
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    _clipEvent(
                        widget,
                        widget->getGeometry(),
                        !widget->isVisible(false));
                }
            }
        }

        void EventLoop::_clipEvent(
            const std::shared_ptr<IWidget>& widget,
            const math::Box2i& clipRect,
            bool clipped)
        {
            const math::Box2i& g = widget->getGeometry();
            clipped |= !g.intersects(clipRect);
            clipped |= !widget->isVisible(false);
            const math::Box2i clipRect2 = g.intersect(clipRect);
            widget->clipEvent(clipRect2, clipped);
            const math::Box2i childrenClipRect =
                widget->getChildrenClipRect().intersect(clipRect2);
            for (const auto& child : widget->getChildren())
            {
                const math::Box2i& childGeometry = child->getGeometry();
                _clipEvent(
                    child,
                    childGeometry.intersect(childrenClipRect),
                    clipped);
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
                    //std::cout << "Draw update: " << widget->getObjectName() << std::endl;
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
                p.fontSystem);
            event.render->setClipRectEnabled(true);
            for (const auto& i : p.topLevelWidgets)
            {
                if (auto widget = i.lock())
                {
                    _drawEvent(
                        widget,
                        math::Box2i(0, 0, p.displaySize.w, p.displaySize.h),
                        event);
                }
            }
            event.render->setClipRectEnabled(false);
        }

        void EventLoop::_drawEvent(
            const std::shared_ptr<IWidget>& widget,
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            const math::Box2i& g = widget->getGeometry();
            if (!widget->isClipped() && g.w() > 0 && g.h() > 0)
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const math::Box2i childrenClipRect =
                    widget->getChildrenClipRect().intersect(drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const math::Box2i& childGeometry = child->getGeometry();
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
            for (auto i = p.topLevelWidgets.rbegin();
                i != p.topLevelWidgets.rend();
                ++i)
            {
                if (auto widget = i->lock())
                {
                    _getUnderCursor(widget, pos, out);
                }
            }
            return out;
        }

        void EventLoop::_getUnderCursor(
            const std::shared_ptr<IWidget>& widget,
            const math::Vector2i& pos,
            std::list<std::shared_ptr<IWidget> >& out)
        {
            if (!widget->isClipped() &&
                widget->isEnabled() &&
                widget->getGeometry().contains(pos))
            {
                for (auto i = widget->getChildren().rbegin();
                    i != widget->getChildren().rend();
                    ++i)
                {
                    _getUnderCursor(*i, pos, out);
                }
                out.push_back(widget);
            }
        }

        void EventLoop::_setHover(const std::shared_ptr<IWidget>& hover)
        {
            TLRENDER_P();
            if (auto widget = p.hover.lock())
            {
                if (hover != widget)
                {
                    //std::cout << "leave: " << widget->getObjectName() << std::endl;
                    widget->mouseLeaveEvent();
                    if (hover)
                    {
                        //std::cout << "enter: " << hover->getObjectName() << std::endl;
                        hover->mouseEnterEvent();
                    }
                }
            }
            else if (hover)
            {
                //std::cout << "enter: " << hover->getObjectName() << std::endl;
                hover->mouseEnterEvent();
            }

            p.hover = hover;

            if (auto widget = p.hover.lock())
            {
                MouseMoveEvent event(
                    p.cursorPos,
                    p.cursorPosPrev);
                widget->mouseMoveEvent(event);
            }
        }

        void EventLoop::_hoverUpdate(MouseMoveEvent& event)
        {
            auto widgets = _getUnderCursor(event.pos);
            while (!widgets.empty())
            {
                if (widgets.front()->hasMouseHover())
                {
                    break;
                }
                widgets.pop_front();
            }
            _setHover(!widgets.empty() ? widgets.front() : nullptr);
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
