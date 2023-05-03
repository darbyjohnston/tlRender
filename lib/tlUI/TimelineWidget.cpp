// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineWidget.h>

#include <tlUI/ScrollWidget.h>

namespace tl
{
    namespace ui
    {
        struct TimelineWidget::Private
        {
            std::shared_ptr<timeline::TimelinePlayer> timelinePlayer;
            bool frameView = true;
            std::function<void(bool)> frameViewCallback;
            bool stopOnScrub = true;
            float mouseWheelScale = 20.F;
            TimelineItemOptions itemOptions;

            std::shared_ptr<ScrollWidget> scrollWidget;
            std::shared_ptr<TimelineItem> timelineItem;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            enum class MouseMode
            {
                None,
                Scroll,
                Scale
            };
            struct MouseData
            {
                math::Vector2i pressPos;
                MouseMode mode = MouseMode::None;
                math::Vector2i scrollPos;
                float scale = 1.F;
                std::chrono::steady_clock::time_point wheelTimer;
            };
            MouseData mouse;
        };

        void TimelineWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineWidget", context, parent);
            TLRENDER_P();

            p.scrollWidget = ScrollWidget::create(
                context,
                ScrollType::Both,
                shared_from_this());

            p.scrollWidget->setScrollPosCallback(
                [this](const math::Vector2i&)
                {
                    _p->frameView = false;
                });
        }

        TimelineWidget::TimelineWidget() :
            _p(new Private)
        {}

        TimelineWidget::~TimelineWidget()
        {}

        std::shared_ptr<TimelineWidget> TimelineWidget::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineWidget>(new TimelineWidget);
            out->_init(context, parent);
            return out;
        }

        void TimelineWidget::setTimelinePlayer(const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer)
        {
            TLRENDER_P();
            if (timelinePlayer == p.timelinePlayer)
                return;
            if (p.timelineItem)
            {
                p.timelineItem->setParent(nullptr);
                p.timelineItem.reset();
            }
            p.timelinePlayer = timelinePlayer;
            if (p.timelinePlayer)
            {
                if (auto context = _context.lock())
                {
                    TimelineItemData itemData;
                    itemData.directory = p.timelinePlayer->getPath().getDirectory();
                    itemData.pathOptions = p.timelinePlayer->getOptions().pathOptions;
                    itemData.ioManager = TimelineIOManager::create(
                        p.timelinePlayer->getOptions().ioOptions,
                        context);

                    p.timelineItem = TimelineItem::create(p.timelinePlayer, itemData, context);
                    p.timelineItem->setStopOnScrub(p.stopOnScrub);
                    p.scrollWidget->setScrollPos(math::Vector2i());
                    p.itemOptions.scale = _getTimelineScale();
                    _setItemOptions(p.timelineItem, p.itemOptions);
                    _setViewport(p.timelineItem, _getTimelineViewport());
                    p.scrollWidget->setWidget(p.timelineItem);
                }
            }
        }

        void TimelineWidget::setViewZoom(float value)
        {
            setViewZoom(value, math::Vector2i(_geometry.w() / 2, _geometry.h() / 2));
        }

        void TimelineWidget::setViewZoom(
            float zoom,
            const math::Vector2i& focus)
        {
            TLRENDER_P();
            _setViewZoom(
                zoom,
                p.itemOptions.scale,
                focus,
                p.scrollWidget->getScrollPos());
        }

        void TimelineWidget::setFrameView(bool value)
        {
            TLRENDER_P();
            if (value == p.frameView)
                return;
            p.frameView = value;
            if (p.frameView)
            {
                _frameView();
            }
            if (p.frameViewCallback)
            {
                p.frameViewCallback(p.frameView);
            }
        }

        void TimelineWidget::setFrameViewCallback(const std::function<void(bool)>& value)
        {
            _p->frameViewCallback = value;
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            TLRENDER_P();
            p.stopOnScrub = value;
            if (p.timelineItem)
            {
                p.timelineItem->setStopOnScrub(p.stopOnScrub);
            }
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            TLRENDER_P();
            p.mouseWheelScale = value;
        }

        const TimelineItemOptions& TimelineWidget::itemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setItemOptions(const TimelineItemOptions& value)
        {
            TLRENDER_P();
            if (value == p.itemOptions)
                return;
            p.itemOptions = value;
            if (p.frameView)
            {
                p.scrollWidget->setScrollPos(math::Vector2i());
                p.itemOptions.scale = _getTimelineScale();
            }
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
            }
        }

        void TimelineWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();

            p.scrollWidget->setGeometry(value);

            if (p.frameView)
            {
                _frameView();
            }
            if (p.timelineItem)
            {
                _setViewport(p.timelineItem, _getTimelineViewport());
            }
        }

        void TimelineWidget::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void TimelineWidget::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void TimelineWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, event.displayScale);

            const int sa = event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);
            _sizeHint.x = sa;
            _sizeHint.y = sa * 2;
        }

        void TimelineWidget::clipEvent(bool clipped, const ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipped, event);
            if (changed && clipped)
            {
                _resetMouse();
            }
        }

        void TimelineWidget::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Scroll:
            {
                const math::Vector2i d = event.pos - p.mouse.pressPos;
                p.scrollWidget->setScrollPos(p.mouse.scrollPos - d);
                setFrameView(false);
                break;
            }
            case Private::MouseMode::Scale:
            {
                const float zoom = p.mouse.scale + (event.pos.x - p.mouse.pressPos.x) * 10.F;
                _setViewZoom(
                    zoom,
                    p.mouse.scale,
                    p.mouse.pressPos,
                    p.mouse.scrollPos);
                break;
            }
            }
        }

        void TimelineWidget::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressPos = event.pos;
            if (event.modifiers & static_cast<int>(KeyModifier::Control))
            {
                p.mouse.mode = Private::MouseMode::Scroll;
            }
            else if (event.modifiers & static_cast<int>(KeyModifier::Alt))
            {
                p.mouse.mode = Private::MouseMode::Scale;
            }
            else
            {
                p.mouse.mode = Private::MouseMode::None;
            }
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Scroll:
            case Private::MouseMode::Scale:
            {
                p.mouse.scrollPos = p.scrollWidget->getScrollPos();
                p.mouse.scale = p.itemOptions.scale;
                break;
            }
            }
        }

        void TimelineWidget::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            switch (event.key)
            {
            case Key::_0:
                event.accept = true;
                setViewZoom(1.F, event.pos);
                break;
            case Key::Equal:
                event.accept = true;
                setViewZoom(p.itemOptions.scale * 2.F, event.pos);
                break;
            case Key::Minus:
                event.accept = true;
                setViewZoom(p.itemOptions.scale / 2.F, event.pos);
                break;
            case Key::Backspace:
                event.accept = true;
                _frameView();
                break;
            }
        }

        void TimelineWidget::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineWidget::_frameView()
        {
            TLRENDER_P();
            p.scrollWidget->setScrollPos(math::Vector2i());
            p.itemOptions.scale = _getTimelineScale();
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
            }
        }

        void TimelineWidget::_setViewZoom(
            float zoomNew,
            float zoomPrev,
            const math::Vector2i& focus,
            const math::Vector2i& scrollPos)
        {
            TLRENDER_P();

            const int w = _geometry.w();
            const int h = _geometry.h();
            const float zoomMin = _getTimelineScale();
            const float zoomMax = w;
            const float zoomClamped = math::clamp(zoomNew, zoomMin, zoomMax);

            const float s = zoomClamped / zoomPrev;
            const math::Vector2i scrollPosNew(
                (scrollPos.x + focus.x) * s - focus.x,
                scrollPos.y);
            const math::Vector2i scrollSize = p.scrollWidget->getScrollSize();
            const math::Vector2i scrollPosClamped(
                math::clamp(scrollPosNew.x, 0, std::max(static_cast<int>(scrollSize.x * s) - w, 0)),
                math::clamp(scrollPosNew.y, 0, std::max(static_cast<int>(scrollSize.y * s) - h, 0)));
            p.scrollWidget->setScrollPos(scrollPosClamped);

            p.itemOptions.scale = zoomClamped;
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
                _setViewport(p.timelineItem, _getTimelineViewport());
            }

            setFrameView(false);
        }

        float TimelineWidget::_getTimelineScale() const
        {
            TLRENDER_P();
            float out = 100.F;
            if (p.timelinePlayer)
            {
                const otime::TimeRange& timeRange = p.timelinePlayer->getTimeRange();
                const double duration = timeRange.duration().rescaled_to(1.0).value();
                if (duration > 0.0)
                {
                    const math::Vector2i& scrollAreaSize = p.scrollWidget->getScrollAreaSize();
                    out = (scrollAreaSize.x - p.size.margin * 2) / duration;
                }
            }
            return out;
        }

        void TimelineWidget::_setItemOptions(
            const std::shared_ptr<IWidget>& widget,
            const TimelineItemOptions& value)
        {
            if (auto item = std::dynamic_pointer_cast<ITimelineItem>(widget))
            {
                item->setOptions(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setItemOptions(child, value);
            }
        }

        math::BBox2i TimelineWidget::_getTimelineViewport() const
        {
            return _p->scrollWidget->getScrollAreaGeometry();
        }

        void TimelineWidget::_setViewport(
            const std::shared_ptr<IWidget>& widget,
            const math::BBox2i& vp)
        {
            if (auto item = std::dynamic_pointer_cast<ITimelineItem>(widget))
            {
                item->setViewport(vp);
            }
            for (const auto& child : widget->getChildren())
            {
                _setViewport(child, vp);
            }
        }

        void TimelineWidget::_resetMouse()
        {
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }
    }
}
