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

            std::shared_ptr<observer::List<std::shared_ptr<IWidget> > > widgets;
            std::shared_ptr<IWidget> activeWidget;
            struct WidgetData
            {
                math::Size2i resolution;
                float scale = 1.F;
                int updates = 0;
            };
            std::map<std::shared_ptr<IWidget>, WidgetData> widgetData;

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

            p.widgets = observer::List<std::shared_ptr<IWidget> >::create();

            p.logTimer = std::chrono::steady_clock::now();

            p.styleChangedObserver = observer::ValueObserver<bool>::create(
                p.style->observeChanged(),
                [this](bool)
                {
                    for (auto widget : _p->widgets->get())
                    {
                        _p->widgetData[widget].updates |= Update::Size;
                        _p->widgetData[widget].updates |= Update::Draw;
                    }
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

        void EventLoop::setWidgetResolution(
            const std::shared_ptr<IWidget>& widget,
            const math::Size2i& value)
        {
            TLRENDER_P();
            auto i = p.widgetData.find(widget);
            if (i != p.widgetData.end() && value == i->second.resolution)
                return;
            p.widgetData[widget].resolution = value;
            p.widgetData[widget].updates |= Update::Size;
            p.widgetData[widget].updates |= Update::Draw;
        }

        void EventLoop::setWidgetScale(
            const std::shared_ptr<IWidget>& widget,
            float value)
        {
            TLRENDER_P();
            auto i = p.widgetData.find(widget);
            if (i != p.widgetData.end() && value == i->second.scale)
                return;
            p.widgetData[widget].scale = value;
            p.widgetData[widget].updates |= Update::Size;
            p.widgetData[widget].updates |= Update::Draw;
        }

        void EventLoop::setKeyFocus(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (value == p.keyFocus.lock())
                return;
            if (auto widget = p.keyFocus.lock())
            {
                widget->keyFocusEvent(false);
                p.widgetData[widget->getTopLevel()].updates |= Update::Draw;
            }
            p.keyFocus = value;
            if (auto widget = p.keyFocus.lock())
            {
                widget->keyFocusEvent(true);
                p.widgetData[widget->getTopLevel()].updates |= Update::Draw;
            }
        }

        void EventLoop::addWidget(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();
            widget->setEventLoop(shared_from_this());
            p.widgets->pushBack(widget);
            p.widgetData[widget].updates |= Update::Size;
            p.widgetData[widget].updates |= Update::Draw;
        }

        void EventLoop::removeWidget(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();

            if (widget == p.activeWidget)
            {
                p.activeWidget.reset();
            }
            {
                auto i = p.widgetData.find(widget);
                if (i != p.widgetData.end())
                {
                    p.widgetData.erase(i);
                }
            }
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
            const size_t i = p.widgets->indexOf(widget);
            if (i != observer::invalidListIndex)
            {
                p.widgets->removeItem(i);
            }
        }

        std::shared_ptr<observer::IList<std::shared_ptr<IWidget> > > EventLoop::observeWidgets() const
        {
            return _p->widgets;
        }

        bool EventLoop::key(
            Key key,
            bool press,
            int modifiers)
        {
            TLRENDER_P();
            p.keyEvent.key = key;
            p.keyEvent.modifiers = modifiers;
            p.keyEvent.pos = p.cursorPos;
            p.keyEvent.accept = false;
            if (press)
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

        void EventLoop::cursorEnter(
            const std::shared_ptr<IWidget>& widget,
            bool enter)
        {
            TLRENDER_P();
            p.activeWidget = enter ? widget : nullptr;
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
            auto widget = p.mousePress.lock();
            if (widget)
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

            if (widget && p.dndCursor)
            {
                p.widgetData[widget->getTopLevel()].updates |= Update::Draw;
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
                    p.widgetData[widget->getTopLevel()].updates |= Update::Draw;
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

            for (const auto& widget : p.widgets->get())
            {
                if (_getSizeUpdate(widget))
                {
                    p.widgetData[widget].updates |= Update::Size;
                }
                if (p.widgetData[widget].updates & Update::Size)
                {
                    _sizeHintEvent(widget);
                    const auto i = p.widgetData.find(widget);
                    if (i != p.widgetData.end())
                    {
                        widget->setGeometry(math::Box2i(
                            0,
                            0,
                            i->second.resolution.w,
                            i->second.resolution.h));
                    }
                    _clipEvent(widget);
                    p.widgetData[widget].updates &= ~static_cast<int>(Update::Size);
                }
                if (_getDrawUpdate(widget))
                {
                    p.widgetData[widget].updates |= Update::Draw;
                }
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
                    if (!text.empty() && !widgets.empty())
                    {
                        p.toolTip = ToolTip::create(
                            text,
                            p.cursorPos,
                            widgets.front()->getTopLevel(),
                            context);
                        p.toolTipPos = p.cursorPos;
                    }
                }
            }

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

        bool EventLoop::hasDrawUpdate(const std::shared_ptr<IWidget>& widget) const
        {
            return _p->widgetData[widget].updates & Update::Draw;
        }

        void EventLoop::draw(
            const std::shared_ptr<IWidget>& widget,
            const std::shared_ptr<timeline::IRender>& render)
        {
            TLRENDER_P();
            _drawEvent(widget, render);
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
            _p->widgetData[widget].updates &= ~static_cast<int>(Update::Draw);
        }

        void EventLoop::_tickEvent()
        {
            TLRENDER_P();
            p.widgetCount = 0;
            TickEvent event(
                p.style,
                p.iconLibrary,
                p.fontSystem);
            for (const auto& widget : p.widgets->get())
            {
                _tickEvent(
                    widget,
                    widget->isVisible(false),
                    widget->isEnabled(false),
                    event);
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

        void EventLoop::_sizeHintEvent(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();
            const auto i = p.widgetData.find(widget);
            if (i != p.widgetData.end())
            {
                SizeHintEvent event(
                    p.style,
                    p.iconLibrary,
                    p.fontSystem,
                    i->second.scale);
                _sizeHintEvent(widget, event);
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

        void EventLoop::_clipEvent(const std::shared_ptr<IWidget>& widget)
        {
            TLRENDER_P();
            _clipEvent(
                widget,
                widget->getGeometry(),
                !widget->isVisible(false));
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

        void EventLoop::_drawEvent(
            const std::shared_ptr<IWidget>& widget,
            const std::shared_ptr<timeline::IRender>& render)
        {
            TLRENDER_P();
            DrawEvent event(
                p.style,
                p.iconLibrary,
                render,
                p.fontSystem);
            event.render->setClipRectEnabled(true);
            const auto i = p.widgetData.find(widget);
            if (i != p.widgetData.end())
            {
                _drawEvent(
                    widget,
                    math::Box2i(0, 0, i->second.resolution.w, i->second.resolution.h),
                    event);
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
            if (p.activeWidget)
            {
                _getUnderCursor(p.activeWidget, pos, out);
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
            if (p.activeWidget)
            {
                _getKeyFocus(p.activeWidget, widgets);
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
            if (p.activeWidget)
            {
                _getKeyFocus(p.activeWidget, widgets);
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
    }
}
