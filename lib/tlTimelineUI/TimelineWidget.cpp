// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineWidget.h>

#include <dtk/ui/ScrollWidget.h>
#include <dtk/gl/GL.h>
#include <dtk/gl/Window.h>

namespace tl
{
    namespace timelineui
    {
        namespace
        {
            const float marginPercentage = .1F;
        }

        struct TimelineWidget::Private
        {
            std::shared_ptr<ItemData> itemData;
            std::shared_ptr<timeline::Player> player;
            std::shared_ptr<dtk::ObservableValue<bool> > editable;
            std::shared_ptr<dtk::ObservableValue<bool> > frameView;
            std::function<void(bool)> frameViewCallback;
            std::shared_ptr<dtk::ObservableValue<bool> > autoScroll;
            dtk::KeyModifier scrollKeyModifier = dtk::KeyModifier::Control;
            float mouseWheelScale = 1.1F;
            std::shared_ptr<dtk::ObservableValue<bool> > stopOnScrub;
            std::shared_ptr<dtk::ObservableValue<bool> > scrub;
            std::shared_ptr<dtk::ObservableValue<OTIO_NS::RationalTime> > timeScrub;
            std::vector<int> frameMarkers;
            std::shared_ptr<dtk::ObservableValue<ItemOptions> > itemOptions;
            std::shared_ptr<dtk::ObservableValue<DisplayOptions> > displayOptions;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            timeline::Playback playback = timeline::Playback::Stop;
            OTIO_NS::RationalTime currentTime = time::invalidTime;
            double scale = 500.0;
            bool sizeInit = true;
            float displayScale = 0.F;

            std::shared_ptr<dtk::gl::Window> window;

            std::shared_ptr<dtk::ScrollWidget> scrollWidget;
            std::shared_ptr<TimelineItem> timelineItem;

            enum class MouseMode
            {
                None,
                Scroll
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                dtk::V2I scrollPos;
                std::chrono::steady_clock::time_point wheelTimer;
            };
            MouseData mouse;

            std::shared_ptr<dtk::ValueObserver<bool> > timelineObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<dtk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > scrubObserver;
            std::shared_ptr<dtk::ValueObserver<OTIO_NS::RationalTime> > timeScrubObserver;
        };

        void TimelineWidget::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::TimelineWidget", parent);
            DTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true, 0, static_cast<int>(p.scrollKeyModifier));

            p.itemData = std::make_shared<ItemData>();
            p.itemData->timeUnitsModel = timeUnitsModel;

            p.editable = dtk::ObservableValue<bool>::create(false);
            p.frameView = dtk::ObservableValue<bool>::create(true);
            p.autoScroll = dtk::ObservableValue<bool>::create(true);
            p.stopOnScrub = dtk::ObservableValue<bool>::create(true);
            p.scrub = dtk::ObservableValue<bool>::create(false);
            p.timeScrub = dtk::ObservableValue<OTIO_NS::RationalTime>::create(time::invalidTime);
            p.itemOptions = dtk::ObservableValue<ItemOptions>::create();
            p.displayOptions = dtk::ObservableValue<DisplayOptions>::create();

            p.window = dtk::gl::Window::create(
                context,
                "tl::timelineui::TimelineWidget",
                dtk::Size2I(1, 1),
                static_cast<int>(dtk::gl::WindowOptions::None));

            p.scrollWidget = dtk::ScrollWidget::create(
                context,
                dtk::ScrollType::Both,
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
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineWidget>(new TimelineWidget);
            out->_init(context, timeUnitsModel, parent);
            return out;
        }

        std::shared_ptr<timeline::Player>& TimelineWidget::getPlayer() const
        {
            return _p->player;
        }

        void TimelineWidget::setPlayer(const std::shared_ptr<timeline::Player>& player)
        {
            DTK_P();
            if (player == p.player)
                return;

            p.itemData->info.clear();
            p.itemData->thumbnails.clear();
            p.itemData->waveforms.clear();
            p.timeRange = time::invalidTimeRange;
            p.playback = timeline::Playback::Stop;
            p.timelineObserver.reset();
            p.playbackObserver.reset();
            p.currentTimeObserver.reset();
            p.scrollWidget->setWidget(nullptr);
            p.timelineItem.reset();

            p.player = player;

            p.scale = _getTimelineScale();
            if (p.player)
            {
                p.timeRange = p.player->getTimeRange();

                p.timelineObserver = dtk::ValueObserver<bool>::create(
                    p.player->getTimeline()->observeTimelineChanges(),
                    [this](bool)
                    {
                        _timelineUpdate();
                    });

                p.playbackObserver = dtk::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        _p->playback = value;
                    });

                p.currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
                    p.player->observeCurrentTime(),
                    [this](const OTIO_NS::RationalTime& value)
                    {
                        _p->currentTime = value;
                        _scrollUpdate();
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

        std::shared_ptr<dtk::IObservableValue<bool> > TimelineWidget::observeEditable() const
        {
            return _p->editable;
        }

        void TimelineWidget::setEditable(bool value)
        {
            DTK_P();
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
            const dtk::Box2I& g = getGeometry();
            setViewZoom(value, dtk::V2I(g.w() / 2, g.h() / 2));
        }

        void TimelineWidget::setViewZoom(
            double zoom,
            const dtk::V2I& focus)
        {
            DTK_P();
            _setViewZoom(
                zoom,
                p.scale,
                focus,
                p.scrollWidget->getScrollPos());
        }

        void TimelineWidget::frameView()
        {
            DTK_P();
            p.scrollWidget->setScrollPos(dtk::V2I());
            const double scale = _getTimelineScale();
            if (scale != p.scale)
            {
                p.scale = scale;
                _setItemScale();
                _setSizeUpdate();
                _setDrawUpdate();
            }
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->frameView->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > TimelineWidget::observeFrameView() const
        {
            return _p->frameView;
        }

        void TimelineWidget::setFrameView(bool value)
        {
            DTK_P();
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

        bool TimelineWidget::hasAutoScroll() const
        {
            return _p->autoScroll->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > TimelineWidget::observeAutoScroll() const
        {
            return _p->autoScroll;
        }

        void TimelineWidget::setAutoScroll(bool value)
        {
            DTK_P();
            if (p.autoScroll->setIfChanged(value))
            {
                _scrollUpdate();
            }
        }

        dtk::KeyModifier TimelineWidget::getScrollKeyModifier() const
        {
            return _p->scrollKeyModifier;
        }

        void TimelineWidget::setScrollKeyModifier(dtk::KeyModifier value)
        {
            DTK_P();
            p.scrollKeyModifier = value;
            _setMousePressEnabled(true, 0, static_cast<int>(p.scrollKeyModifier));
        }

        float TimelineWidget::getMouseWheelScale() const
        {
            return _p->mouseWheelScale;
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            DTK_P();
            p.mouseWheelScale = value;
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->stopOnScrub->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > TimelineWidget::observeStopOnScrub() const
        {
            return _p->stopOnScrub;
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            DTK_P();
            if (p.stopOnScrub->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    p.timelineItem->setStopOnScrub(value);
                }
            }
        }

        std::shared_ptr<dtk::IObservableValue<bool> > TimelineWidget::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<dtk::IObservableValue<OTIO_NS::RationalTime> > TimelineWidget::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        const std::vector<int>& TimelineWidget::getFrameMarkers() const
        {
            return _p->frameMarkers;
        }

        void TimelineWidget::setFrameMarkers(const std::vector<int>& value)
        {
            DTK_P();
            if (value == p.frameMarkers)
                return;
            p.frameMarkers = value;
            if (p.timelineItem)
            {
                p.timelineItem->setFrameMarkers(value);
            }
        }

        const ItemOptions& TimelineWidget::getItemOptions() const
        {
            return _p->itemOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<ItemOptions> > TimelineWidget::observeItemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setItemOptions(const ItemOptions& value)
        {
            DTK_P();
            if (p.itemOptions->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    _setItemOptions(p.timelineItem, value);
                }
            }
        }

        const DisplayOptions& TimelineWidget::getDisplayOptions() const
        {
            return _p->displayOptions->get();
        }

        std::shared_ptr<dtk::IObservableValue<DisplayOptions> > TimelineWidget::observeDisplayOptions() const
        {
            return _p->displayOptions;
        }

        void TimelineWidget::setDisplayOptions(const DisplayOptions& value)
        {
            DTK_P();
            const DisplayOptions prev = p.displayOptions->get();
            if (p.displayOptions->setIfChanged(value))
            {
                if (prev.thumbnailHeight != value.thumbnailHeight)
                {
                    p.itemData->thumbnails.clear();
                }
                if (prev.waveformWidth != value.waveformWidth ||
                    prev.waveformHeight != value.waveformHeight ||
                    prev.waveformPrim != value.waveformPrim)
                {
                    p.itemData->waveforms.clear();
                }
                if (p.timelineItem)
                {
                    _setDisplayOptions(p.timelineItem, value);
                }
            }
        }

        std::vector<dtk::Box2I> TimelineWidget::getTrackGeom() const
        {
            return _p->timelineItem->getTrackGeom();
        }

        void TimelineWidget::setGeometry(const dtk::Box2I& value)
        {
            const bool changed = value != getGeometry();
            IWidget::setGeometry(value);
            DTK_P();
            p.scrollWidget->setGeometry(value);
            if (p.sizeInit || (changed && p.frameView->get()))
            {
                p.sizeInit = false;
                frameView();
            }
            else if (p.timelineItem &&
                p.timelineItem->getSizeHint().w <
                p.scrollWidget->getViewport().w())
            {
                setFrameView(true);
                frameView();
            }
        }

        void TimelineWidget::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const dtk::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
        }

        void TimelineWidget::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            DTK_P();
            const int b = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
            const int sa = event.style->getSizeRole(dtk::SizeRole::ScrollArea, event.displayScale);
            dtk::Size2I sizeHint;
            sizeHint.w = sa;
            //! \bug This assumes the scroll bars are hidden.
            sizeHint.h = p.timelineItem ? (p.timelineItem->getMinimumHeight() + b * 2) : sa;
            _setSizeHint(sizeHint);
            p.sizeInit |= event.displayScale != p.displayScale;
            p.displayScale = event.displayScale;
        }

        void TimelineWidget::mouseMoveEvent(dtk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            DTK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Scroll:
            {
                const dtk::V2I d = event.pos - _getMousePressPos();
                p.scrollWidget->setScrollPos(p.mouse.scrollPos - d);
                setFrameView(false);
                break;
            }
            default: break;
            }
        }

        void TimelineWidget::mousePressEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            DTK_P();
            if (p.itemOptions->get().inputEnabled &&
                0 == event.button &&
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

        void TimelineWidget::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            DTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::scrollEvent(dtk::ScrollEvent& event)
        {
            DTK_P();
            if (p.itemOptions->get().inputEnabled)
            {
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
        }

        void TimelineWidget::keyPressEvent(dtk::KeyEvent& event)
        {
            DTK_P();
            if (p.itemOptions->get().inputEnabled &&
                0 == event.modifiers)
            {
                switch (event.key)
                {
                case dtk::Key::Equal:
                    event.accept = true;
                    setViewZoom(p.scale * 2.F, event.pos);
                    break;
                case dtk::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.scale / 2.F, event.pos);
                    break;
                case dtk::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                    break;
                default: break;
                }
            }
        }

        void TimelineWidget::keyReleaseEvent(dtk::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineWidget::_releaseMouse()
        {
            IWidget::_releaseMouse();
            DTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::_setViewZoom(
            double zoomNew,
            double zoomPrev,
            const dtk::V2I& focus,
            const dtk::V2I& scrollPos)
        {
            DTK_P();
            const int w = getGeometry().w();
            const double zoomMin = _getTimelineScale();
            const double zoomMax = _getTimelineScaleMax();
            const double zoomClamped = dtk::clamp(zoomNew, zoomMin, zoomMax);
            if (zoomClamped != p.scale)
            {
                p.scale = zoomClamped;
                _setItemScale();
                const double s = zoomClamped / zoomPrev;
                const dtk::V2I scrollPosNew(
                    (scrollPos.x + focus.x) * s - focus.x,
                    scrollPos.y);
                p.scrollWidget->setScrollPos(scrollPosNew, false);

                setFrameView(zoomNew <= zoomMin);
            }
        }

        double TimelineWidget::_getTimelineScale() const
        {
            DTK_P();
            double out = 1.0;
            if (p.player)
            {
                const OTIO_NS::TimeRange& timeRange = p.player->getTimeRange();
                const double duration = timeRange.duration().rescaled_to(1.0).value();
                if (duration > 0.0)
                {
                    const dtk::Box2I scrollViewport = p.scrollWidget->getViewport();
                    out = scrollViewport.w() / duration;
                }
            }
            return out;
        }

        double TimelineWidget::_getTimelineScaleMax() const
        {
            DTK_P();
            double out = 1.0;
            if (p.player)
            {
                const dtk::Box2I scrollViewport = p.scrollWidget->getViewport();
                const OTIO_NS::TimeRange& timeRange = p.player->getTimeRange();
                const double duration = timeRange.duration().rescaled_to(1.0).value();
                if (duration < 1.0)
                {
                    if (duration > 0.0)
                    {
                        out = scrollViewport.w() / duration;
                    }
                }
                else
                {
                    out = scrollViewport.w();
                }
            }
            return out;
        }

        void TimelineWidget::_setItemScale()
        {
            DTK_P();
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

        void TimelineWidget::_setDisplayOptions(
            const std::shared_ptr<IWidget>& widget,
            const DisplayOptions& value)
        {
            if (auto item = std::dynamic_pointer_cast<IItem>(widget))
            {
                item->setDisplayOptions(value);
            }
            for (const auto& child : widget->getChildren())
            {
                _setDisplayOptions(child, value);
            }
        }

        void TimelineWidget::_scrollUpdate()
        {
            DTK_P();
            if (p.timelineItem &&
                p.autoScroll->get() &&
                !p.scrub->get() &&
                Private::MouseMode::None == p.mouse.mode)
            {
                const int pos = p.timelineItem->timeToPos(p.currentTime);
                const dtk::Box2I vp = p.scrollWidget->getViewport();
                const int margin = vp.w() * marginPercentage;
                if (pos < (vp.min.x + margin) || pos >(vp.max.x - margin))
                {
                    const int offset = pos < (vp.min.x + margin) ? (vp.min.x + margin) : (vp.max.x - margin);
                    const OTIO_NS::RationalTime t = p.currentTime - p.timeRange.start_time();
                    dtk::V2I scrollPos = p.scrollWidget->getScrollPos();
                    scrollPos.x = getGeometry().min.x - offset + t.rescaled_to(1.0).value() * p.scale;
                    p.scrollWidget->setScrollPos(scrollPos);
                }
            }
        }

        void TimelineWidget::_timelineUpdate()
        {
            DTK_P();

            const dtk::V2I scrollPos = p.scrollWidget->getScrollPos();

            p.scrubObserver.reset();
            p.timeScrubObserver.reset();
            p.scrollWidget->setWidget(nullptr);
            p.timelineItem.reset();

            if (p.player)
            {
                if (auto context = getContext())
                {
                    p.itemData->speed = p.player->getDefaultSpeed();
                    p.itemData->directory = p.player->getPath().getDirectory();
                    p.itemData->options = p.player->getOptions();
                    p.timelineItem = TimelineItem::create(
                        context,
                        p.player,
                        p.player->getTimeline()->getTimeline()->tracks(),
                        p.scale,
                        p.itemOptions->get(),
                        p.displayOptions->get(),
                        p.itemData,
                        p.window);
                    p.timelineItem->setEditable(p.editable->get());
                    p.timelineItem->setStopOnScrub(p.stopOnScrub->get());
                    p.timelineItem->setFrameMarkers(p.frameMarkers);
                    p.scrollWidget->setScrollPos(scrollPos);
                    p.scrollWidget->setWidget(p.timelineItem);

                    p.scrubObserver = dtk::ValueObserver<bool>::create(
                        p.timelineItem->observeScrub(),
                        [this](bool value)
                        {
                            _p->scrub->setIfChanged(value);
                            _scrollUpdate();
                        });

                    p.timeScrubObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
                        p.timelineItem->observeTimeScrub(),
                        [this](const OTIO_NS::RationalTime& value)
                        {
                            _p->timeScrub->setIfChanged(value);
                        });
                }
            }
        }
    }
}
