// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/ScrollWidget.h>

#include <tlGL/GL.h>
#include <tlGL/GLFWWindow.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineWidget::Private
        {
            std::shared_ptr<ItemData> itemData;
            std::shared_ptr<timeline::Player> player;
            std::shared_ptr<observer::ValueObserver<bool> > timelineObserver;
            std::shared_ptr<observer::Value<bool> > editable;
            std::shared_ptr<observer::Value<bool> > frameView;
            std::function<void(bool)> frameViewCallback;
            ui::KeyModifier scrollKeyModifier = ui::KeyModifier::Control;
            std::shared_ptr<observer::Value<bool> > stopOnScrub;
            float mouseWheelScale = 1.1F;
            double scale = 500.0;
            std::shared_ptr<observer::Value<ItemOptions> > itemOptions;
            bool sizeInit = true;

            std::shared_ptr<gl::GLFWWindow> window;

            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<TimelineItem> timelineItem;

            enum class MouseMode
            {
                None,
                Scroll
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                math::Vector2i scrollPos;
                std::chrono::steady_clock::time_point wheelTimer;
            };
            MouseData mouse;
        };

        void TimelineWidget::_init(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineWidget", context, parent);
            TLRENDER_P();

            _setMouseHover(true);
            _setMousePress(true, 0, static_cast<int>(p.scrollKeyModifier));

            p.itemData = std::make_shared<ItemData>();
            p.itemData->timeUnitsModel = timeUnitsModel;

            p.editable = observer::Value<bool>::create(false);
            p.frameView = observer::Value<bool>::create(true);
            p.stopOnScrub = observer::Value<bool>::create(true);
            p.itemOptions = observer::Value<ItemOptions>::create();

            p.window = gl::GLFWWindow::create(
                "tl::timelineui::TimelineWidget",
                math::Size2i(1, 1),
                context,
                static_cast<int>(gl::GLFWWindowOptions::None));

            p.scrollWidget = ui::ScrollWidget::create(
                context,
                ui::ScrollType::Both,
                shared_from_this());
            p.scrollWidget->setScrollEventsEnabled(false);
            p.scrollWidget->setBorder(false);
        }

        TimelineWidget::TimelineWidget() :
            _p(new Private)
        {}

        TimelineWidget::~TimelineWidget()
        {}

        std::shared_ptr<TimelineWidget> TimelineWidget::create(
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineWidget>(new TimelineWidget);
            out->_init(timeUnitsModel, context, parent);
            return out;
        }

        void TimelineWidget::setPlayer(const std::shared_ptr<timeline::Player>& player)
        {
            TLRENDER_P();
            if (player == p.player)
                return;

            p.itemData->info.clear();
            p.itemData->thumbnails.clear();
            p.itemData->waveforms.clear();
            p.timelineObserver.reset();
            p.scrollWidget->setWidget(nullptr);
            p.timelineItem.reset();

            p.player = player;

            p.scale = _getTimelineScale();
            if (p.player)
            {
                p.timelineObserver = observer::ValueObserver<bool>::create(
                    p.player->getTimeline()->observeTimelineChanges(),
                    [this](bool)
                    {
                        _timelineUpdate();
                    });
            }
            else
            {
                _timelineUpdate();
            }
        }

        bool TimelineWidget::isEditable() const
        {
            return _p->editable->get();
        }

        std::shared_ptr<observer::IValue<bool> > TimelineWidget::observeEditable() const
        {
            return _p->editable;
        }

        void TimelineWidget::setEditable(bool value)
        {
            TLRENDER_P();
            if (p.editable->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    p.timelineItem->setEditable(value);
                }
            }
        }

        void TimelineWidget::setViewZoom(double value)
        {
            setViewZoom(value, math::Vector2i(_geometry.w() / 2, _geometry.h() / 2));
        }

        void TimelineWidget::setViewZoom(
            double zoom,
            const math::Vector2i& focus)
        {
            TLRENDER_P();
            _setViewZoom(
                zoom,
                p.scale,
                focus,
                p.scrollWidget->getScrollPos());
        }

        void TimelineWidget::frameView()
        {
            TLRENDER_P();
            p.scrollWidget->setScrollPos(math::Vector2i());
            const double scale = _getTimelineScale();
            if (scale != p.scale)
            {
                p.scale = scale;
                _setItemScale();
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<observer::IValue<bool> > TimelineWidget::observeFrameView() const
        {
            return _p->frameView;
        }

        void TimelineWidget::setFrameView(bool value)
        {
            TLRENDER_P();
            if (p.frameView->setIfChanged(value))
            {
                if (value)
                {
                    frameView();
                }
            }
        }

        bool TimelineWidget::areScrollBarsVisible() const
        {
            return _p->scrollWidget->areScrollBarsVisible();
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->scrollWidget->setScrollBarsVisible(value);
        }

        ui::KeyModifier TimelineWidget::getScrollKeyModifier() const
        {
            return _p->scrollKeyModifier;
        }

        void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
        {
            TLRENDER_P();
            p.scrollKeyModifier = value;
            _setMousePress(true, 0, static_cast<int>(p.scrollKeyModifier));
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->stopOnScrub->get();
        }

        std::shared_ptr<observer::IValue<bool> > TimelineWidget::observeStopOnScrub() const
        {
            return _p->stopOnScrub;
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            TLRENDER_P();
            if (p.stopOnScrub->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    p.timelineItem->setStopOnScrub(value);
                }
            }
        }

        float TimelineWidget::getMouseWheelScale() const
        {
            return _p->mouseWheelScale;
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            TLRENDER_P();
            p.mouseWheelScale = value;
        }

        const ItemOptions& TimelineWidget::getItemOptions() const
        {
            return _p->itemOptions->get();
        }

        std::shared_ptr<observer::IValue<ItemOptions> > TimelineWidget::observeItemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setItemOptions(const ItemOptions& value)
        {
            TLRENDER_P();
            if (p.itemOptions->setIfChanged(value))
            {
                p.itemData->info.clear();
                p.itemData->thumbnails.clear();
                p.itemData->waveforms.clear();
                if (p.timelineItem)
                {
                    _setItemOptions(p.timelineItem, value);
                }
            }
        }

        void TimelineWidget::setGeometry(const math::Box2i& value)
        {
            const bool changed = value != _geometry;
            IWidget::setGeometry(value);
            TLRENDER_P();
            p.scrollWidget->setGeometry(value);
            if (p.sizeInit)
            {
                p.sizeInit = false;
                frameView();
            }
            else if (changed && p.frameView->get())
            {
                frameView();
            }
        }

        void TimelineWidget::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const ui::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
        }

        void TimelineWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();
            const int sa = event.style->getSizeRole(ui::SizeRole::ScrollArea, _displayScale);
            _sizeHint.w = sa;
            _sizeHint.h = sa * 2;
        }

        void TimelineWidget::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Scroll:
            {
                const math::Vector2i d = event.pos - _mouse.pressPos;
                p.scrollWidget->setScrollPos(p.mouse.scrollPos - d);
                setFrameView(false);
                break;
            }
            default: break;
            }
        }

        void TimelineWidget::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            if (0 == event.button &&
                event.modifiers & static_cast<int>(p.scrollKeyModifier))
            {
                takeKeyFocus();
                p.mouse.mode = Private::MouseMode::Scroll;
                p.mouse.scrollPos = p.scrollWidget->getScrollPos();
            }
            else
            {
                p.mouse.mode = Private::MouseMode::None;
            }
        }

        void TimelineWidget::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::scrollEvent(ui::ScrollEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            if (event.value.y > 0)
            {
                const double zoom = p.scale * p.mouseWheelScale;
                setViewZoom(zoom, event.pos);
            }
            else
            {
                const double zoom = p.scale / p.mouseWheelScale;
                setViewZoom(zoom, event.pos);
            }
        }

        void TimelineWidget::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                switch (event.key)
                {
                case ui::Key::_0:
                    event.accept = true;
                    setViewZoom(1.F, event.pos);
                    break;
                case ui::Key::Equal:
                    event.accept = true;
                    setViewZoom(p.scale * 2.F, event.pos);
                    break;
                case ui::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.scale / 2.F, event.pos);
                    break;
                case ui::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                    break;
                default: break;
                }
            }
        }

        void TimelineWidget::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineWidget::_releaseMouse()
        {
            IWidget::_releaseMouse();
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::_setViewZoom(
            double zoomNew,
            double zoomPrev,
            const math::Vector2i& focus,
            const math::Vector2i& scrollPos)
        {
            TLRENDER_P();
            const int w = _geometry.w();
            const double zoomMin = _getTimelineScale();
            const double zoomMax = w;
            const double zoomClamped = math::clamp(zoomNew, zoomMin, zoomMax);
            if (zoomClamped != p.scale)
            {
                p.scale = zoomClamped;
                _setItemScale();
                const double s = zoomClamped / zoomPrev;
                const math::Vector2i scrollPosNew(
                    (scrollPos.x + focus.x) * s - focus.x,
                    scrollPos.y);
                p.scrollWidget->setScrollPos(scrollPosNew, false);

                setFrameView(zoomNew <= zoomMin);
            }
        }

        double TimelineWidget::_getTimelineScale() const
        {
            TLRENDER_P();
            double out = 100.0;
            if (p.player)
            {
                const otime::TimeRange& timeRange = p.player->getTimeRange();
                const double duration = timeRange.duration().rescaled_to(1.0).value();
                if (duration > 0.0)
                {
                    const math::Box2i scrollViewport = p.scrollWidget->getViewport();
                    out = scrollViewport.w() / duration;
                }
            }
            return out;
        }

        void TimelineWidget::_setItemScale()
        {
            TLRENDER_P();
            p.itemData->waveforms.clear();
            if (p.timelineItem)
            {
                _setItemScale(p.timelineItem, p.scale);
            }
        }

        void TimelineWidget::_setItemScale(
            const std::shared_ptr<IWidget>& widget,
            double value)
        {
            if (auto item = std::dynamic_pointer_cast<IItem>(widget))
            {
                item->setScale(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setItemScale(child, value);
            }
        }

        void TimelineWidget::_setItemOptions(
            const std::shared_ptr<IWidget>& widget,
            const ItemOptions& value)
        {
            if (auto item = std::dynamic_pointer_cast<IItem>(widget))
            {
                item->setOptions(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setItemOptions(child, value);
            }
        }

        void TimelineWidget::_timelineUpdate()
        {
            TLRENDER_P();

            const math::Vector2i scrollPos = p.scrollWidget->getScrollPos();

            p.scrollWidget->setWidget(nullptr);
            p.timelineItem.reset();

            if (p.player)
            {
                if (auto context = _context.lock())
                {
                    p.itemData->speed = p.player->getDefaultSpeed();
                    p.itemData->directory = p.player->getPath().getDirectory();
                    p.itemData->options = p.player->getOptions();
                    p.timelineItem = TimelineItem::create(
                        p.player,
                        p.player->getTimeline()->getTimeline()->tracks(),
                        p.scale,
                        p.itemOptions->get(),
                        p.itemData,
                        p.window,
                        context);
                    p.timelineItem->setEditable(p.editable->get());
                    p.timelineItem->setStopOnScrub(p.stopOnScrub->get());
                    p.scrollWidget->setScrollPos(scrollPos);
                    p.scrollWidget->setWidget(p.timelineItem);
                }
            }
        }
    }
}
