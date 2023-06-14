// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineWidget.h>

#include <tlUI/ScrollWidget.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineWidget::Private
        {
            std::shared_ptr<timeline::TimeUnitsModel> timeUnitsModel;
            std::shared_ptr<timeline::Player> player;
            bool frameView = true;
            std::function<void(bool)> frameViewCallback;
            ui::KeyModifier scrollKeyModifier = ui::KeyModifier::Control;
            bool stopOnScrub = true;
            float mouseWheelScale = 1.1F;
            double scale = 500.0;
            ItemOptions itemOptions;

            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<TimelineItem> timelineItem;

            struct SizeData
            {
                bool init = true;
                int margin = 0;
            };
            SizeData size;

            enum class MouseMode
            {
                None,
                Scroll
            };
            struct MouseData
            {
                math::Vector2i pressPos;
                MouseMode mode = MouseMode::None;
                math::Vector2i scrollPos;
                std::chrono::steady_clock::time_point wheelTimer;
            };
            MouseData mouse;
        };

        void TimelineWidget::_init(
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::TimelineWidget", context, parent);
            TLRENDER_P();

            p.timeUnitsModel = timeUnitsModel;

            p.scrollWidget = ui::ScrollWidget::create(
                context,
                ui::ScrollType::Both,
                shared_from_this());
            p.scrollWidget->setBorder(false);
            //p.scrollWidget->setMarginRole(ui::SizeRole::MarginInside);

            p.scrollWidget->setScrollPosCallback(
                [this](const math::Vector2i&)
                {
                    setFrameView(false);
                });
        }

        TimelineWidget::TimelineWidget() :
            _p(new Private)
        {}

        TimelineWidget::~TimelineWidget()
        {}

        std::shared_ptr<TimelineWidget> TimelineWidget::create(
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
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
            if (p.timelineItem)
            {
                p.timelineItem->setParent(nullptr);
                p.timelineItem.reset();
            }
            p.player = player;
            if (p.player)
            {
                if (auto context = _context.lock())
                {
                    ItemData itemData;
                    itemData.directory = p.player->getPath().getDirectory();
                    itemData.pathOptions = p.player->getOptions().pathOptions;
                    itemData.ioManager = IOManager::create(
                        p.player->getOptions().ioOptions,
                        context);
                    itemData.timeUnitsModel = p.timeUnitsModel;

                    p.timelineItem = TimelineItem::create(p.player, itemData, context);
                    p.timelineItem->setStopOnScrub(p.stopOnScrub);
                    p.scrollWidget->setScrollPos(math::Vector2i());
                    p.scale = _getTimelineScale();
                    _setItemScale(p.timelineItem, p.scale);
                    _setItemOptions(p.timelineItem, p.itemOptions);
                    p.scrollWidget->setWidget(p.timelineItem);
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
            p.scale = _getTimelineScale();
            if (p.timelineItem)
            {
                _setItemScale(p.timelineItem, p.scale);
            }
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void TimelineWidget::setFrameView(bool value)
        {
            TLRENDER_P();
            if (value == p.frameView)
                return;
            if (value)
            {
                frameView();
            }
            p.frameView = value;
            if (p.frameViewCallback)
            {
                p.frameViewCallback(p.frameView);
            }
        }

        void TimelineWidget::setFrameViewCallback(const std::function<void(bool)>& value)
        {
            _p->frameViewCallback = value;
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->scrollWidget->setScrollBarsVisible(value);
        }

        void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
        {
            _p->scrollKeyModifier = value;
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

        const ItemOptions& TimelineWidget::getItemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setItemOptions(const ItemOptions& value)
        {
            TLRENDER_P();
            if (value == p.itemOptions)
                return;
            p.itemOptions = value;
            if (p.timelineItem)
            {
                _setItemOptions(p.timelineItem, p.itemOptions);
            }
        }

        void TimelineWidget::setGeometry(const math::BBox2i& value)
        {
            const bool changed = value != _geometry;
            IWidget::setGeometry(value);
            TLRENDER_P();
            p.scrollWidget->setGeometry(value);
            if (p.size.init)
            {
                p.size.init = false;
                frameView();
            }
            else if (changed && p.frameView)
            {
                frameView();
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

        void TimelineWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);

            const int sa = event.style->getSizeRole(ui::SizeRole::ScrollArea, event.displayScale);
            _sizeHint.x = sa;
            _sizeHint.y = sa * 2;
        }

        void TimelineWidget::clipEvent(
            const math::BBox2i& clipRect,
            bool clipped,
            const ui::ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipRect, clipped, event);
            if (changed && clipped)
            {
                _resetMouse();
            }
        }

        void TimelineWidget::mouseMoveEvent(ui::MouseMoveEvent& event)
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
            default: break;
            }
        }

        void TimelineWidget::mousePressEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            takeKeyFocus();
            p.mouse.pressPos = event.pos;
            if (event.modifiers & static_cast<int>(p.scrollKeyModifier))
            {
                p.mouse.mode = Private::MouseMode::Scroll;
            }
            else
            {
                p.mouse.mode = Private::MouseMode::None;
            }
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Scroll:
                p.mouse.scrollPos = p.scrollWidget->getScrollPos();
                break;
            default: break;
            }
        }

        void TimelineWidget::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::scrollEvent(ui::ScrollEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            if (event.dy > 0)
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

        void TimelineWidget::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineWidget::_setViewZoom(
            double zoomNew,
            double zoomPrev,
            const math::Vector2i& focus,
            const math::Vector2i& scrollPos)
        {
            TLRENDER_P();
            const int w = _geometry.w();
            const int h = _geometry.h();
            const double zoomMin = _getTimelineScale();
            const double zoomMax = w;
            const double zoomClamped = math::clamp(zoomNew, zoomMin, zoomMax);
            if (zoomClamped != p.scale)
            {
                p.scale = zoomClamped;
                if (p.timelineItem)
                {
                    _setItemScale(p.timelineItem, p.scale);
                }
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
                    const math::BBox2i scrollViewport = p.scrollWidget->getViewport();
                    out = (scrollViewport.w() - p.size.margin * 2) / duration;
                }
            }
            return out;
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

        void TimelineWidget::_resetMouse()
        {
            TLRENDER_P();
            p.mouse.mode = Private::MouseMode::None;
        }
    }
}
