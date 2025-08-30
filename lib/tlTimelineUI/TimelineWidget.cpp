// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineWidget.h>

#include <feather-tk/ui/ScrollWidget.h>
#include <feather-tk/gl/GL.h>
#include <feather-tk/gl/Window.h>

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
            std::shared_ptr<ftk::ObservableValue<bool> > editable;
            std::shared_ptr<ftk::ObservableValue<bool> > frameView;
            std::function<void(bool)> frameViewCallback;
            std::shared_ptr<ftk::ObservableValue<bool> > scrollBarsVisible;
            std::shared_ptr<ftk::ObservableValue<bool> > autoScroll;
            std::pair<int, ftk::KeyModifier> scrollBinding = std::make_pair(1, ftk::KeyModifier::Control);
            float mouseWheelScale = 1.1F;
            std::shared_ptr<ftk::ObservableValue<bool> > stopOnScrub;
            std::shared_ptr<ftk::ObservableValue<bool> > scrub;
            std::shared_ptr<ftk::ObservableValue<OTIO_NS::RationalTime> > timeScrub;
            std::vector<int> frameMarkers;
            std::shared_ptr<ftk::ObservableValue<ItemOptions> > itemOptions;
            std::shared_ptr<ftk::ObservableValue<DisplayOptions> > displayOptions;
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            timeline::Playback playback = timeline::Playback::Stop;
            OTIO_NS::RationalTime currentTime = time::invalidTime;
            double scale = 500.0;
            bool sizeInit = true;
            float displayScale = 0.F;

            std::shared_ptr<ftk::gl::Window> window;

            std::shared_ptr<ftk::ScrollWidget> scrollWidget;
            std::shared_ptr<TimelineItem> timelineItem;

            enum class MouseMode
            {
                None,
                Scroll
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                ftk::V2I scrollPos;
            };
            MouseData mouse;

            std::shared_ptr<ftk::ValueObserver<bool> > timelineObserver;
            std::shared_ptr<ftk::ValueObserver<timeline::Playback> > playbackObserver;
            std::shared_ptr<ftk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<ftk::ValueObserver<bool> > scrubObserver;
            std::shared_ptr<ftk::ValueObserver<OTIO_NS::RationalTime> > timeScrubObserver;
        };

        void TimelineWidget::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::TimelineWidget", parent);
            FTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true, p.scrollBinding.first, static_cast<int>(p.scrollBinding.second));

            p.itemData = std::make_shared<ItemData>();
            p.itemData->timeUnitsModel = timeUnitsModel;

            p.editable = ftk::ObservableValue<bool>::create(false);
            p.frameView = ftk::ObservableValue<bool>::create(true);
            p.scrollBarsVisible = ftk::ObservableValue<bool>::create(true);
            p.autoScroll = ftk::ObservableValue<bool>::create(true);
            p.stopOnScrub = ftk::ObservableValue<bool>::create(true);
            p.scrub = ftk::ObservableValue<bool>::create(false);
            p.timeScrub = ftk::ObservableValue<OTIO_NS::RationalTime>::create(time::invalidTime);
            p.itemOptions = ftk::ObservableValue<ItemOptions>::create();
            p.displayOptions = ftk::ObservableValue<DisplayOptions>::create();

            p.window = ftk::gl::Window::create(
                context,
                "tl::timelineui::TimelineWidget",
                ftk::Size2I(1, 1),
                static_cast<int>(ftk::gl::WindowOptions::None));

            p.scrollWidget = ftk::ScrollWidget::create(
                context,
                ftk::ScrollType::Both,
                shared_from_this());
            p.scrollWidget->setScrollBarsVisible(p.scrollBarsVisible->get());
            p.scrollWidget->setScrollEventsEnabled(false);
            p.scrollWidget->setBorder(false);
        }

        TimelineWidget::TimelineWidget() :
            _p(new Private)
        {}

        TimelineWidget::~TimelineWidget()
        {}

        std::shared_ptr<TimelineWidget> TimelineWidget::create(
            const std::shared_ptr<ftk::Context>& context,
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
            FTK_P();
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

                p.timelineObserver = ftk::ValueObserver<bool>::create(
                    p.player->getTimeline()->observeTimelineChanges(),
                    [this](bool)
                    {
                        _timelineUpdate();
                    });

                p.playbackObserver = ftk::ValueObserver<timeline::Playback>::create(
                    p.player->observePlayback(),
                    [this](timeline::Playback value)
                    {
                        _p->playback = value;
                    });

                p.currentTimeObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
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

        std::shared_ptr<ftk::IObservableValue<bool> > TimelineWidget::observeEditable() const
        {
            return _p->editable;
        }

        void TimelineWidget::setEditable(bool value)
        {
            FTK_P();
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
            const ftk::Box2I& g = getGeometry();
            setViewZoom(value, ftk::V2I(g.w() / 2, g.h() / 2));
        }

        void TimelineWidget::setViewZoom(
            double zoom,
            const ftk::V2I& focus)
        {
            FTK_P();
            _setViewZoom(
                zoom,
                p.scale,
                focus,
                p.scrollWidget->getScrollPos());
        }

        void TimelineWidget::frameView()
        {
            FTK_P();
            p.scrollWidget->setScrollPos(ftk::V2I());
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

        std::shared_ptr<ftk::IObservableValue<bool> > TimelineWidget::observeFrameView() const
        {
            return _p->frameView;
        }

        void TimelineWidget::setFrameView(bool value)
        {
            FTK_P();
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
            return _p->scrollBarsVisible->get();
        }

        std::shared_ptr<ftk::IObservableValue<bool> > TimelineWidget::observeScrollBarsVisible() const
        {
            return _p->scrollBarsVisible;
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            FTK_P();
            if (p.scrollBarsVisible->setIfChanged(value))
            {
                _p->scrollWidget->setScrollBarsVisible(value);
            }
        }

        bool TimelineWidget::hasAutoScroll() const
        {
            return _p->autoScroll->get();
        }

        std::shared_ptr<ftk::IObservableValue<bool> > TimelineWidget::observeAutoScroll() const
        {
            return _p->autoScroll;
        }

        void TimelineWidget::setAutoScroll(bool value)
        {
            FTK_P();
            if (p.autoScroll->setIfChanged(value))
            {
                _scrollUpdate();
            }
        }

        void TimelineWidget::setScrollBinding(int button, ftk::KeyModifier modifier)
        {
            FTK_P();
            p.scrollBinding = std::make_pair(button, modifier);
            _setMousePressEnabled(true, button, static_cast<int>(modifier));
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            _p->mouseWheelScale = value;
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->stopOnScrub->get();
        }

        std::shared_ptr<ftk::IObservableValue<bool> > TimelineWidget::observeStopOnScrub() const
        {
            return _p->stopOnScrub;
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            FTK_P();
            if (p.stopOnScrub->setIfChanged(value))
            {
                if (p.timelineItem)
                {
                    p.timelineItem->setStopOnScrub(value);
                }
            }
        }

        std::shared_ptr<ftk::IObservableValue<bool> > TimelineWidget::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<ftk::IObservableValue<OTIO_NS::RationalTime> > TimelineWidget::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        const std::vector<int>& TimelineWidget::getFrameMarkers() const
        {
            return _p->frameMarkers;
        }

        void TimelineWidget::setFrameMarkers(const std::vector<int>& value)
        {
            FTK_P();
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

        std::shared_ptr<ftk::IObservableValue<ItemOptions> > TimelineWidget::observeItemOptions() const
        {
            return _p->itemOptions;
        }

        void TimelineWidget::setItemOptions(const ItemOptions& value)
        {
            FTK_P();
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

        std::shared_ptr<ftk::IObservableValue<DisplayOptions> > TimelineWidget::observeDisplayOptions() const
        {
            return _p->displayOptions;
        }

        void TimelineWidget::setDisplayOptions(const DisplayOptions& value)
        {
            FTK_P();
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
                _scrollUpdate();
            }
        }

        std::vector<ftk::Box2I> TimelineWidget::getTrackGeom() const
        {
            return _p->timelineItem->getTrackGeom();
        }

        void TimelineWidget::setGeometry(const ftk::Box2I& value)
        {
            const bool changed = value != getGeometry();
            IWidget::setGeometry(value);
            FTK_P();
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
            const ftk::TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
        }

        void TimelineWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            FTK_P();
            _setSizeHint(p.scrollWidget->getSizeHint());
            p.sizeInit |= event.displayScale != p.displayScale;
            p.displayScale = event.displayScale;
        }

        void TimelineWidget::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            FTK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::Scroll:
            {
                const ftk::V2I d = event.pos - _getMousePressPos();
                p.scrollWidget->setScrollPos(p.mouse.scrollPos - d);
                setFrameView(false);
                break;
            }
            default: break;
            }
        }

        void TimelineWidget::mousePressEvent(ftk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            FTK_P();
            if (p.itemOptions->get().inputEnabled &&
                p.scrollBinding.first == event.button &&
                event.modifiers & static_cast<int>(p.scrollBinding.second))
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

        void TimelineWidget::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            FTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::scrollEvent(ftk::ScrollEvent& event)
        {
            FTK_P();
            if (p.itemOptions->get().inputEnabled)
            {
                event.accept = true;
                const double newZoom =
                    event.value.y > 0 ?
                    p.scale * p.mouseWheelScale :
                    p.scale / p.mouseWheelScale;
                setViewZoom(newZoom, event.pos);
            }
        }

        void TimelineWidget::keyPressEvent(ftk::KeyEvent& event)
        {
            FTK_P();
            if (p.itemOptions->get().inputEnabled &&
                0 == event.modifiers)
            {
                switch (event.key)
                {
                case ftk::Key::Equals:
                    event.accept = true;
                    setViewZoom(p.scale * 2.F, event.pos);
                    break;
                case ftk::Key::Minus:
                    event.accept = true;
                    setViewZoom(p.scale / 2.F, event.pos);
                    break;
                case ftk::Key::Backspace:
                    event.accept = true;
                    setFrameView(true);
                    break;
                default: break;
                }
            }
        }

        void TimelineWidget::keyReleaseEvent(ftk::KeyEvent& event)
        {
            event.accept = true;
        }

        void TimelineWidget::_releaseMouse()
        {
            IWidget::_releaseMouse();
            FTK_P();
            p.mouse.mode = Private::MouseMode::None;
        }

        void TimelineWidget::_setViewZoom(
            double zoomNew,
            double zoomPrev,
            const ftk::V2I& focus,
            const ftk::V2I& scrollPos)
        {
            FTK_P();
            const int w = getGeometry().w();
            const double zoomMin = _getTimelineScale();
            const double zoomMax = _getTimelineScaleMax();
            const double zoomClamped = ftk::clamp(zoomNew, zoomMin, zoomMax);
            if (zoomClamped != p.scale)
            {
                p.scale = zoomClamped;
                _setItemScale();
                const double s = zoomClamped / zoomPrev;
                const ftk::V2I scrollPosNew(
                    (scrollPos.x + focus.x) * s - focus.x,
                    scrollPos.y);
                p.scrollWidget->setScrollPos(scrollPosNew, false);

                setFrameView(zoomNew <= zoomMin);
            }
        }

        double TimelineWidget::_getTimelineScale() const
        {
            FTK_P();
            double out = 1.0;
            if (p.player)
            {
                const OTIO_NS::TimeRange& timeRange = p.player->getTimeRange();
                const double duration = timeRange.duration().rescaled_to(1.0).value();
                if (duration > 0.0)
                {
                    const ftk::Box2I scrollViewport = p.scrollWidget->getViewport();
                    out = scrollViewport.w() / duration;
                }
            }
            return out;
        }

        double TimelineWidget::_getTimelineScaleMax() const
        {
            FTK_P();
            double out = 1.0;
            if (p.player)
            {
                const ftk::Box2I scrollViewport = p.scrollWidget->getViewport();
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
            FTK_P();
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
            FTK_P();
            if (p.timelineItem &&
                p.autoScroll->get() &&
                !p.scrub->get() &&
                Private::MouseMode::None == p.mouse.mode)
            {
                const int pos = p.timelineItem->timeToPos(p.currentTime);
                const ftk::Box2I vp = p.scrollWidget->getViewport();
                const int margin = vp.w() * marginPercentage;
                if (pos < (vp.min.x + margin) || pos >(vp.max.x - margin))
                {
                    const int offset = pos < (vp.min.x + margin) ? (vp.min.x + margin) : (vp.max.x - margin);
                    const OTIO_NS::RationalTime t = p.currentTime - p.timeRange.start_time();
                    ftk::V2I scrollPos = p.scrollWidget->getScrollPos();
                    scrollPos.x = getGeometry().min.x - offset + t.rescaled_to(1.0).value() * p.scale;
                    p.scrollWidget->setScrollPos(scrollPos);
                }
            }
            p.scrollWidget->setScrollType(p.displayOptions->get().minimize ?
                ftk::ScrollType::Horizontal :
                ftk::ScrollType::Both);
        }

        void TimelineWidget::_timelineUpdate()
        {
            FTK_P();

            const ftk::V2I scrollPos = p.scrollWidget->getScrollPos();

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

                    p.scrubObserver = ftk::ValueObserver<bool>::create(
                        p.timelineItem->observeScrub(),
                        [this](bool value)
                        {
                            _p->scrub->setIfChanged(value);
                            _scrollUpdate();
                        });

                    p.timeScrubObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
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
