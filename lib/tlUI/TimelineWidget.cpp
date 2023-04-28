// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineWidget.h>

#include <tlUI/ScrollArea.h>

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
            ui::TimelineItemOptions itemOptions;

            std::shared_ptr<ui::ScrollArea> scrollArea;
            std::shared_ptr<ui::TimelineItem> timelineItem;

            struct SizeData
            {
                int margin = 0;
            };
            SizeData size;

            bool mouseInside = false;
            math::Vector2i mousePos;
            math::Vector2i mousePressPos;
            enum class MouseMode
            {
                None,
                Scroll,
                Scale
            };
            MouseMode mouseMode = MouseMode::None;
            math::Vector2i mouseScrollPos;
            float mouseScale = 1.F;
            std::chrono::steady_clock::time_point mouseWheelTimer;
        };

        void TimelineWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineWidget", context, parent);
            TLRENDER_P();

            p.scrollArea = ui::ScrollArea::create(
                context,
                ScrollAreaType::Both,
                shared_from_this());
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
                    ui::TimelineItemData itemData;
                    itemData.directory = p.timelinePlayer->getPath().getDirectory();
                    itemData.pathOptions = p.timelinePlayer->getOptions().pathOptions;
                    itemData.ioManager = ui::TimelineIOManager::create(
                        p.timelinePlayer->getOptions().ioOptions,
                        context);

                    p.timelineItem = ui::TimelineItem::create(p.timelinePlayer, itemData, context);
                    p.timelineItem->setStopOnScrub(p.stopOnScrub);
                    _setScrollPos(math::Vector2i());
                    p.itemOptions.scale = _timelineScale();
                    _setItemOptions(p.timelineItem, p.itemOptions);
                    _setViewport(p.timelineItem, _timelineViewport());
                    p.timelineItem->setParent(p.scrollArea);
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
                p.scrollArea->getScrollPos());
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

        const ui::TimelineItemOptions& TimelineWidget::itemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setItemOptions(const ui::TimelineItemOptions& value)
        {
            TLRENDER_P();
            if (value == p.itemOptions)
                return;
            p.itemOptions = value;
            if (p.frameView)
            {
                _setScrollPos(math::Vector2i());
                p.itemOptions.scale = _timelineScale();
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

            p.scrollArea->setGeometry(value);

            const math::Vector2i scrollSize = p.scrollArea->getScrollSize();
            if (p.frameView || scrollSize.x < _geometry.w())
            {
                _frameView();
            }
            if (p.timelineItem)
            {
                _setViewport(p.timelineItem, _timelineViewport());
            }
        }

        void TimelineWidget::sizeEvent(const SizeEvent& event)
        {
            IWidget::sizeEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall) * event.contentScale;

            const int sa = event.style->getSizeRole(SizeRole::ScrollArea);
            _sizeHint.x = sa;
            _sizeHint.y = sa * 2;
        }

        void TimelineWidget::enterEvent()
        {
            TLRENDER_P();
            p.mouseInside = true;
        }

        void TimelineWidget::leaveEvent()
        {
            TLRENDER_P();
            p.mouseInside = false;
        }

        void TimelineWidget::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mousePos = event.pos;
            switch (p.mouseMode)
            {
            case Private::MouseMode::Scroll:
            {
                const math::Vector2i d = p.mousePos - p.mousePressPos;
                _setScrollPos(p.mouseScrollPos - d);
                setFrameView(false);
                break;
            }
            case Private::MouseMode::Scale:
            {
                const float zoom = p.mouseScale + (p.mousePos.x - p.mousePressPos.x) * 10.F;
                _setViewZoom(
                    zoom,
                    p.mouseScale,
                    p.mousePressPos,
                    p.mouseScrollPos);
                break;
            }
            }
        }

        void TimelineWidget::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mousePressPos = p.mousePos;
            if (event.modifiers & static_cast<int>(KeyModifier::Control))
            {
                p.mouseMode = Private::MouseMode::Scroll;
            }
            else if (event.modifiers & static_cast<int>(KeyModifier::Alt))
            {
                p.mouseMode = Private::MouseMode::Scale;
            }
            else
            {
                p.mouseMode = Private::MouseMode::None;
            }
            switch (p.mouseMode)
            {
            case Private::MouseMode::Scroll:
            case Private::MouseMode::Scale:
            {
                p.mouseScrollPos = p.scrollArea->getScrollPos();
                p.mouseScale = p.itemOptions.scale;
                break;
            }
            }
        }

        void TimelineWidget::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouseMode = Private::MouseMode::None;
        }

        void TimelineWidget::_setScrollPos(const math::Vector2i& value)
        {
            TLRENDER_P();
            const int w = _geometry.w();
            const int h = _geometry.h();
            const math::Vector2i scrollSize = p.scrollArea->getScrollSize();
            const math::Vector2i clamped(
                math::clamp(value.x, 0, std::max(scrollSize.x - w, 0)),
                math::clamp(value.y, 0, std::max(scrollSize.y - h, 0)));
            p.scrollArea->setScrollPos(clamped);
        }

        void TimelineWidget::_frameView()
        {
            TLRENDER_P();
            _setScrollPos(math::Vector2i());
            p.itemOptions.scale = _timelineScale();
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
            const float zoomMin = _timelineScale();
            const float zoomMax = w;
            const float zoomClamped = math::clamp(zoomNew, zoomMin, zoomMax);

            const float s = zoomClamped / zoomPrev;
            const math::Vector2i scrollPosNew(
                (scrollPos.x + focus.x) * s - focus.x,
                scrollPos.y);
            const math::Vector2i scrollSize = p.scrollArea->getScrollSize();
            const math::Vector2i scrollPosClamped(
                math::clamp(scrollPosNew.x, 0, std::max(static_cast<int>(scrollSize.x * s) - w, 0)),
                math::clamp(scrollPosNew.y, 0, std::max(static_cast<int>(scrollSize.y * s) - h, 0)));
            p.scrollArea->setScrollPos(scrollPosClamped);

            p.itemOptions.scale = zoomClamped;
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
                _setViewport(p.timelineItem, _timelineViewport());
            }

            setFrameView(false);
        }

        float TimelineWidget::_timelineScale() const
        {
            TLRENDER_P();
            float out = 100.F;
            if (p.timelinePlayer)
            {
                const otime::TimeRange& timeRange = p.timelinePlayer->getTimeRange();
                const double duration = timeRange.duration().rescaled_to(1.0).value();
                if (duration > 0.0)
                {
                    out = (_geometry.w() - p.size.margin * 2) / duration;
                }
            }
            return out;
        }

        void TimelineWidget::_setItemOptions(
            const std::shared_ptr<ui::IWidget>& widget,
            const ui::TimelineItemOptions& value)
        {
            if (auto item = std::dynamic_pointer_cast<ui::ITimelineItem>(widget))
            {
                item->setOptions(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setItemOptions(child, value);
            }
        }

        math::BBox2i TimelineWidget::_timelineViewport() const
        {
            return _geometry;
        }

        void TimelineWidget::_setViewport(
            const std::shared_ptr<ui::IWidget>& widget,
            const math::BBox2i& vp)
        {
            if (auto item = std::dynamic_pointer_cast<ui::ITimelineItem>(widget))
            {
                item->setViewport(vp);
            }
            for (const auto& child : widget->getChildren())
            {
                _setViewport(child, vp);
            }
        }
    }
}
