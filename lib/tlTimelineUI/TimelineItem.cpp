// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItemPrivate.h>

#include <tlTimelineUI/AudioClipItem.h>
#include <tlTimelineUI/GapItem.h>
#include <tlTimelineUI/VideoClipItem.h>

#include <tlTimeline/Edit.h>
#include <tlTimeline/Util.h>

#include <dtk/ui/DrawUtil.h>
#include <dtk/ui/ScrollArea.h>
#include <dtk/core/Context.h>
#include <dtk/core/Format.h>

namespace tl
{
    namespace timelineui
    {
        void TimelineItem::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<dtk::gl::Window>& window,
            const std::shared_ptr<IWidget>& parent)
        {
            const OTIO_NS::TimeRange timeRange = player->getTimeRange();
            const OTIO_NS::TimeRange trimmedRange(
                OTIO_NS::RationalTime(0.0, timeRange.duration().rate()),
                timeRange.duration());
            IItem::_init(
                context,
                "tl::timelineui::TimelineItem",
                timeRange,
                trimmedRange,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            DTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true, 0, 0);

            p.player = player;

            p.scrub = dtk::ObservableValue<bool>::create(false);
            p.timeScrub = dtk::ObservableValue<OTIO_NS::RationalTime>::create(time::invalidTime);

            p.thumbnailGenerator = ThumbnailGenerator::create(
                context->getSystem<ThumbnailSystem>()->getCache(),
                context,
                window);

            const auto otioTimeline = p.player->getTimeline()->getTimeline();
            int stackIndex = 0;
            for (const auto& child : otioTimeline->tracks()->children())
            {
                if (auto otioTrack = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(child))
                {
                    Private::Track track;
                    track.index = p.tracks.size();
                    std::string trackLabel = otioTrack->name();
                    if (OTIO_NS::Track::Kind::video == otioTrack->kind())
                    {
                        track.type = TrackType::Video;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Video Track";
                        }
                    }
                    else if (OTIO_NS::Track::Kind::audio == otioTrack->kind())
                    {
                        track.type = TrackType::Audio;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Audio Track";
                        }
                    }
                    track.timeRange = otioTrack->trimmed_range();
                    track.enabledButton = dtk::ToolButton::create(
                        context,
                        shared_from_this());
                    track.enabledButton->setIcon("Hidden");
                    track.enabledButton->setCheckedIcon("Visible");
                    track.enabledButton->setCheckedRole(dtk::ColorRole::None);
                    track.enabledButton->setCheckable(true);
                    track.enabledButton->setChecked(otioTrack->enabled());
                    track.enabledButton->setCheckedCallback(
                        [this, stackIndex](bool value)
                        {
                            _setTrackEnabled(stackIndex, value);
                        });
                    track.enabledButton->setAcceptsKeyFocus(false);
                    track.enabledButton->setTooltip("Toggle the enabled state");
                    track.label = dtk::Label::create(
                        context,
                        trackLabel,
                        shared_from_this());
                    track.label->setMarginRole(dtk::SizeRole::MarginInside);
                    track.label->setEnabled(otioTrack->enabled());
                    track.durationLabel = dtk::Label::create(
                        context,
                        shared_from_this());
                    track.durationLabel->setMarginRole(dtk::SizeRole::MarginInside);
                    track.durationLabel->setEnabled(otioTrack->enabled());

                    for (const auto& child : otioTrack->children())
                    {
                        std::shared_ptr<IBasicItem> item;
                        if (auto clip = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(child))
                        {
                            switch (track.type)
                            {
                            case TrackType::Video:
                                item = VideoClipItem::create(
                                    context,
                                    clip,
                                    scale,
                                    options,
                                    displayOptions,
                                    itemData,
                                    p.thumbnailGenerator,
                                    shared_from_this());
                                break;
                            case TrackType::Audio:
                                item = AudioClipItem::create(
                                    context,
                                    clip,
                                    scale,
                                    options,
                                    displayOptions,
                                    itemData,
                                    p.thumbnailGenerator,
                                    shared_from_this());
                                break;
                            default: break;
                            }
                        }
                        else if (auto gap = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Gap>(child))
                        {
                            item = GapItem::create(
                                context,
                                TrackType::Video == track.type ?
                                    dtk::ColorRole::VideoGap :
                                    dtk::ColorRole::AudioGap,
                                gap,
                                scale,
                                options,
                                displayOptions,
                                itemData,
                                shared_from_this());
                        }
                        if (item)
                        {
                            item->setEnabled(otioTrack->enabled());
                            track.items.push_back(item);
                        }
                    }

                    p.tracks.push_back(track);
                }
                ++stackIndex;
            }

            _tracksUpdate();
            _textUpdate();

            p.currentTimeObserver = dtk::ValueObserver<OTIO_NS::RationalTime>::create(
                p.player->observeCurrentTime(),
                [this](const OTIO_NS::RationalTime& value)
                {
                    _p->currentTime = value;
                    _setDrawUpdate();
                });

            p.inOutRangeObserver = dtk::ValueObserver<OTIO_NS::TimeRange>::create(
                p.player->observeInOutRange(),
                [this](const OTIO_NS::TimeRange value)
                {
                    _p->inOutRange = value;
                    _setDrawUpdate();
                });

            p.cacheInfoObserver = dtk::ValueObserver<timeline::PlayerCacheInfo>::create(
                p.player->observeCacheInfo(),
                [this](const timeline::PlayerCacheInfo& value)
                {
                    _p->cacheInfo = value;
                    _setDrawUpdate();
                });
        }

        TimelineItem::TimelineItem() :
            _p(new Private)
        {}

        TimelineItem::~TimelineItem()
        {}

        std::shared_ptr<TimelineItem> TimelineItem::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<dtk::gl::Window>& window,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
            out->_init(
                context,
                player,
                stack,
                scale,
                options,
                displayOptions,
                itemData,
                window,
                parent);
            return out;
        }

        void TimelineItem::setEditable(bool value)
        {
            _p->editable = value;
        }

        void TimelineItem::setStopOnScrub(bool value)
        {
            _p->stopOnScrub = value;
        }

        std::shared_ptr<dtk::IObservableValue<bool> > TimelineItem::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<dtk::IObservableValue<OTIO_NS::RationalTime> > TimelineItem::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        void TimelineItem::setFrameMarkers(const std::vector<int>& value)
        {
            DTK_P();
            if (value == p.frameMarkers)
                return;
            p.frameMarkers = value;
            _setDrawUpdate();
        }

        int TimelineItem::getMinimumHeight() const
        {
            return _p->minimumHeight;
        }

        std::vector<dtk::Box2I> TimelineItem::getTrackGeom() const
        {
            DTK_P();
            std::vector<dtk::Box2I> out;
            for (const auto& track : p.tracks)
            {
                out.push_back(track.geom);
            }
            return out;
        }

        void TimelineItem::setDisplayOptions(const DisplayOptions& value)
        {
            const bool changed = value != _displayOptions;
            IItem::setDisplayOptions(value);
            DTK_P();
            if (changed)
            {
                p.size.init = true;
                _tracksUpdate();
            }
        }

        void TimelineItem::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            DTK_P();

            float y =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border +
                value.min.y;
            for (auto& track : p.tracks)
            {
                const bool visible = _isTrackVisible(track.index);

                dtk::Size2I buttonSizeHint;
                dtk::Size2I labelSizeHint;
                dtk::Size2I durationSizeHint;
                int trackInfoHeight = 0;
                if (visible && _displayOptions.trackInfo)
                {
                    buttonSizeHint = track.enabledButton->getSizeHint();
                    labelSizeHint = track.label->getSizeHint();
                    durationSizeHint = track.durationLabel->getSizeHint();
                    trackInfoHeight = std::max(
                        buttonSizeHint.h,
                        std::max(
                            labelSizeHint.h,
                            durationSizeHint.h));
                }
                track.enabledButton->setGeometry(dtk::Box2I(
                    value.min.x,
                    y + trackInfoHeight / 2 - buttonSizeHint.h / 2,
                    buttonSizeHint.w,
                    buttonSizeHint.h));
                track.label->setGeometry(dtk::Box2I(
                    value.min.x + buttonSizeHint.w + p.size.spacing,
                    y + trackInfoHeight / 2 - labelSizeHint.h / 2,
                    labelSizeHint.w,
                    labelSizeHint.h));
                track.durationLabel->setGeometry(dtk::Box2I(
                    value.min.x + track.size.w - durationSizeHint.w,
                    y + trackInfoHeight / 2 - durationSizeHint.h / 2,
                    durationSizeHint.w,
                    durationSizeHint.h));

                for (const auto& item : track.items)
                {
                    const auto i = std::find_if(
                        p.mouse.items.begin(),
                        p.mouse.items.end(),
                        [item](const std::shared_ptr<Private::MouseItemData>& value)
                        {
                            return item == value->p;
                        });
                    if (i != p.mouse.items.end())
                    {
                        continue;
                    }
                    const OTIO_NS::TimeRange& timeRange = item->getTimeRange();
                    dtk::Size2I sizeHint;
                    if (visible)
                    {
                        sizeHint = item->getSizeHint();
                    }
                    item->setGeometry(dtk::Box2I(
                        value.min.x +
                        timeRange.start_time().rescaled_to(1.0).value() * _scale,
                        y + std::max(labelSizeHint.h, durationSizeHint.h),
                        sizeHint.w,
                        track.clipHeight));
                }

                track.geom = dtk::Box2I(
                    value.min.x,
                    y,
                    track.size.w,
                    visible ? track.size.h : 0);

                if (visible)
                {
                    y += track.size.h;
                }
            }

            if (auto scrollArea = getParentT<dtk::ScrollArea>())
            {
                p.size.scrollPos = scrollArea->getScrollPos();
            }
        }

        void TimelineItem::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            DTK_P();

            const bool displayScaleChanged = event.displayScale != p.size.displayScale;
            if (displayScaleChanged || p.size.init)
            {
                p.size.init = false;
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(dtk::SizeRole::MarginInside, event.displayScale);
                p.size.spacing = event.style->getSizeRole(dtk::SizeRole::SpacingSmall, event.displayScale);
                p.size.border = event.style->getSizeRole(dtk::SizeRole::Border, event.displayScale);
                p.size.handle = event.style->getSizeRole(dtk::SizeRole::Handle, event.displayScale);
                p.size.fontInfo = dtk::FontInfo(
                    _displayOptions.monoFont,
                    _displayOptions.fontSize * event.displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
            }

            int tracksHeight = 0;
            bool minimumTrackHeightInit = true;
            int minimumTrackHeight = 0;
            for (int i = 0; i < p.tracks.size(); ++i)
            {
                auto& track = p.tracks[i];
                const bool visible = _isTrackVisible(track.index);

                track.size.w = track.timeRange.duration().rescaled_to(1.0).value() * _scale;
                track.size.h = 0;
                track.clipHeight = 0;
                if (visible)
                {
                    for (const auto& item : track.items)
                    {
                        const dtk::Size2I& sizeHint = item->getSizeHint();
                        track.size.h = std::max(track.size.h, sizeHint.h);
                    }
                    track.clipHeight = track.size.h;
                    if (_displayOptions.trackInfo)
                    {
                        track.size.h += std::max(
                            track.enabledButton->getSizeHint().h,
                            std::max(
                                track.label->getSizeHint().h,
                                track.durationLabel->getSizeHint().h));
                    }
                    tracksHeight += track.size.h;
                    if (minimumTrackHeightInit)
                    {
                        minimumTrackHeightInit = false;
                        minimumTrackHeight = track.size.h;
                    }
                }
            }

            _setSizeHint(dtk::Size2I(
                _timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border +
                tracksHeight));

            p.minimumHeight =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border +
                minimumTrackHeight;
        }

        void TimelineItem::drawOverlayEvent(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            IItem::drawOverlayEvent(drawRect, event);
            DTK_P();

            const dtk::Box2I& g = getGeometry();

            int y =
                p.size.scrollPos.y +
                g.min.y;
            int h =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4;
            event.render->drawRect(
                dtk::Box2I(g.min.x, y, g.w(), h),
                event.style->getColorRole(dtk::ColorRole::Window));

            y = y + h;
            h = p.size.border;
            event.render->drawRect(
                dtk::Box2I(g.min.x, y, g.w(), h),
                event.style->getColorRole(dtk::ColorRole::Border));

            _drawInOutPoints(drawRect, event);
            _drawTimeTicks(drawRect, event);
            _drawFrameMarkers(drawRect, event);
            _drawTimeLabels(drawRect, event);
            _drawCacheInfo(drawRect, event);
            _drawCurrentTime(drawRect, event);

            if (p.mouse.currentDropTarget >= 0 &&
                p.mouse.currentDropTarget < p.mouse.dropTargets.size())
            {
                const auto& dt = p.mouse.dropTargets[p.mouse.currentDropTarget];
                event.render->drawRect(
                    dt.draw,
                    event.style->getColorRole(dtk::ColorRole::Green));
            }
        }

        void TimelineItem::mouseMoveEvent(dtk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            DTK_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::CurrentTime:
            {
                const OTIO_NS::RationalTime time = posToTime(event.pos.x);
                p.timeScrub->setIfChanged(time);
                p.player->seek(time);
                break;
            }
            case Private::MouseMode::Item:
            {
                if (!p.mouse.items.empty())
                {
                    for (const auto& item : p.mouse.items)
                    {
                        const dtk::Box2I& g = item->geometry;
                        item->p->setGeometry(dtk::Box2I(
                            g.min + _getMousePos() - _getMousePressPos(),
                            g.size()));
                    }
                    
                    int dropTarget = -1;
                    for (size_t i = 0; i < p.mouse.dropTargets.size(); ++i)
                    {
                        if (dtk::contains(p.mouse.dropTargets[i].mouse, event.pos))
                        {
                            dropTarget = i;
                            break;
                        }
                    }
                    if (dropTarget != p.mouse.currentDropTarget)
                    {
                        for (const auto& item : p.mouse.items)
                        {
                            item->p->setSelectRole(
                                dropTarget != -1 ?
                                dtk::ColorRole::Green :
                                dtk::ColorRole::Checked);
                        }
                        p.mouse.currentDropTarget = dropTarget;
                        _setDrawUpdate();
                    }
                }
                break;
            }
            default: break;
            }
        }

        void TimelineItem::mousePressEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            DTK_P();
            if (_options.inputEnabled &&
                0 == event.button &&
                0 == event.modifiers)
            {
                takeKeyFocus();

                p.mouse.mode = Private::MouseMode::None;

                const dtk::Box2I& g = getGeometry();
                if (p.editable)
                {
                    for (int i = 0; i < p.tracks.size(); ++i)
                    {
                        if (_isTrackVisible(i))
                        {
                            const auto& items = p.tracks[i].items;
                            for (int j = 0; j < items.size(); ++j)
                            {
                                const auto& item = items[j];
                                if (dtk::contains(item->getGeometry(), event.pos))
                                {
                                    p.mouse.mode = Private::MouseMode::Item;
                                    p.mouse.items.push_back(
                                        std::make_shared<Private::MouseItemData>(item, j, i));
                                    p.mouse.dropTargets = p.getDropTargets(g, j, i);
                                    moveToFront(item);
                                    if (_options.editAssociatedClips)
                                    {
                                        if (auto associated = p.getAssociated(item, j, i))
                                        {
                                            p.mouse.items.push_back(
                                                std::make_shared<Private::MouseItemData>(associated, j, i));
                                            moveToFront(associated);
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        if (!p.mouse.items.empty())
                        {
                            break;
                        }
                    }
                }

                if (p.mouse.items.empty())
                {
                    p.mouse.mode = Private::MouseMode::CurrentTime;
                    if (p.stopOnScrub)
                    {
                        p.player->setPlayback(timeline::Playback::Stop);
                    }
                    const OTIO_NS::RationalTime time = posToTime(event.pos.x);
                    p.scrub->setIfChanged(true);
                    p.timeScrub->setIfChanged(time);
                    p.player->seek(time);
                }
            }
        }

        void TimelineItem::mouseReleaseEvent(dtk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            DTK_P();
            p.scrub->setIfChanged(false);
            p.mouse.mode = Private::MouseMode::None;
            if (!p.mouse.items.empty() && p.mouse.currentDropTarget != -1)
            {
                const auto& dropTarget = p.mouse.dropTargets[p.mouse.currentDropTarget];
                std::vector<timeline::MoveData> moveData;
                for (const auto& item : p.mouse.items)
                {
                    const int track = dropTarget.track + (item->track - p.mouse.items[0]->track);
                    moveData.push_back({ item->track, item->index, track, dropTarget.index });
                    item->p->hide();
                }
                auto otioTimeline = timeline::move(
                    p.player->getTimeline()->getTimeline().value,
                    moveData);
                p.player->getTimeline()->setTimeline(otioTimeline);
            }
            p.mouse.items.clear();
            if (!p.mouse.dropTargets.empty())
            {
                p.mouse.dropTargets.clear();
                _setDrawUpdate();
            }
            p.mouse.currentDropTarget = -1;
        }

        /*void TimelineItem::keyPressEvent(dtk::KeyEvent& event)
        {
            DTK_P();
            if (isEnabled() &&
                _options.inputEnabled &&
                0 == event.modifiers)
            {
                switch (event.key)
                {
                default: break;
                }
            }
        }

        void TimelineItem::keyReleaseEvent(dtk::KeyEvent& event)
        {
            event.accept = true;
        }*/

        void TimelineItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
            _setSizeUpdate();
            _setDrawUpdate();
        }

        void TimelineItem::_releaseMouse()
        {
            DTK_P();
            IWidget::_releaseMouse();
            p.mouse.items.clear();
        }

        bool TimelineItem::_isTrackVisible(int index) const
        {
            return
                _displayOptions.tracks.empty() ||
                std::find(
                    _displayOptions.tracks.begin(),
                    _displayOptions.tracks.end(),
                    index) != _displayOptions.tracks.end();
        }

        void TimelineItem::_setTrackEnabled(int stackIndex, bool enabled)
        {
            DTK_P();
            auto otioTimeline = timeline::copy(p.player->getTimeline()->getTimeline().value);
            const auto& children = otioTimeline->tracks()->children();
            if (stackIndex >= 0 && stackIndex < children.size())
            {
                if (auto item = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(children[stackIndex]))
                {
                    item->set_enabled(enabled);
                }
            }
            p.player->getTimeline()->setTimeline(otioTimeline);
        }

        void TimelineItem::_drawInOutPoints(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            DTK_P();
            if (!time::compareExact(_p->inOutRange, time::invalidTimeRange) &&
                !time::compareExact(_p->inOutRange, _timeRange))
            {
                const dtk::Box2I& g = getGeometry();
                const dtk::Color4F color(.4F, .5F, .9F);

                switch (_displayOptions.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = timeToPos(_p->inOutRange.start_time());
                    const int x1 = timeToPos(_p->inOutRange.end_time_exclusive());
                    const dtk::Box2I box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(box, color);
                    break;
                }
                case InOutDisplay::OutsideRange:
                {
                    int x0 = timeToPos(_timeRange.start_time());
                    int x1 = timeToPos(_p->inOutRange.start_time());
                    dtk::Box2I box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(box, color);
                    x0 = timeToPos(_p->inOutRange.end_time_exclusive());
                    x1 = timeToPos(_timeRange.end_time_exclusive());
                    box = dtk::Box2I(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(box, color);
                    break;
                }
                default: break;
                }
            }
        }

        dtk::Size2I TimelineItem::_getLabelMaxSize(
            const std::shared_ptr<dtk::FontSystem>& fontSystem) const
        {
            DTK_P();
            const std::string labelMax = _data->timeUnitsModel->getLabel(_timeRange.duration());
            const dtk::Size2I labelMaxSize = fontSystem->getSize(labelMax, p.size.fontInfo);
            return labelMaxSize;
        }

        void TimelineItem::_getTimeTicks(
            const std::shared_ptr<dtk::FontSystem>& fontSystem,
            double& seconds,
            int& tick)
        {
            DTK_P();
            const int w = getSizeHint().w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            const int secondsTick = 1.0 / duration * w;
            const int minutesTick = 60.0 / duration * w;
            const int hoursTick = 3600.0 / duration * w;
            const dtk::Size2I labelMaxSize = _getLabelMaxSize(fontSystem);
            const int distanceMin = p.size.border + p.size.margin + labelMaxSize.w;
            seconds = 0.0;
            tick = 0;
            if (secondsTick >= distanceMin)
            {
                seconds = 1.0;
                tick = secondsTick;
            }
            else if (minutesTick >= distanceMin)
            {
                seconds = 60.0;
                tick = minutesTick;
            }
            else if (hoursTick >= distanceMin)
            {
                seconds = 3600.0;
                tick = hoursTick;
            }
        }

        void TimelineItem::_drawTimeTicks(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            DTK_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const dtk::Box2I& g = getGeometry();
                const int w = getSizeHint().w;
                const float duration = _timeRange.duration().rescaled_to(1.0).value();
                const int frameTick = 1.0 / _timeRange.duration().value() * w;
                if (duration > 0.0 && frameTick >= p.size.handle)
                {
                    dtk::TriMesh2F mesh;
                    size_t i = 1;
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = 1.0 / _timeRange.duration().rate();
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const dtk::Box2I box(
                            g.min.x +
                            t / duration * w,
                            p.size.scrollPos.y +
                            g.min.y +
                            p.size.margin +
                            p.size.fontMetrics.lineHeight,
                            p.size.border,
                            p.size.margin +
                            p.size.border * 4);
                        if (dtk::intersects(box, drawRect))
                        {
                            mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                            mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                            mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                            mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                            i += 4;
                        }
                    }
                    if (!mesh.v.empty())
                    {
                        event.render->drawMesh(
                            mesh,
                            event.style->getColorRole(dtk::ColorRole::Button));
                    }
                }

                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (duration > 0.0 && seconds > 0.0 && tick > 0)
                {
                    dtk::TriMesh2F mesh;
                    size_t i = 1;
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = seconds;
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const dtk::Box2I box(
                            g.min.x +
                            t / duration * w,
                            p.size.scrollPos.y +
                            g.min.y,
                            p.size.border,
                            p.size.margin +
                            p.size.fontMetrics.lineHeight +
                            p.size.margin +
                            p.size.border * 4);
                        if (dtk::intersects(box, drawRect))
                        {
                            mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                            mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                            mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                            mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                            i += 4;
                        }
                    }
                    if (!mesh.v.empty())
                    {
                        event.render->drawMesh(
                            mesh,
                            event.style->getColorRole(dtk::ColorRole::Button));
                    }
                }
            }
        }

        void TimelineItem::_drawFrameMarkers(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            DTK_P();
            const dtk::Box2I& g = getGeometry();
            const double rate = _timeRange.duration().rate();
            const dtk::Color4F color(.6F, .4F, .2F);
            for (const auto& frameMarker : p.frameMarkers)
            {
                const dtk::Box2I g2(
                    timeToPos(OTIO_NS::RationalTime(frameMarker, rate)),
                    p.size.scrollPos.y +
                    g.min.y,
                    p.size.border * 2,
                    p.size.margin +
                    p.size.fontMetrics.lineHeight +
                    p.size.margin +
                    p.size.border * 4);
                if (dtk::intersects(g2, drawRect))
                {
                    event.render->drawRect(g2, color);
                }
            }
        }

        void TimelineItem::_drawTimeLabels(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            DTK_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const dtk::Box2I& g = getGeometry();
                const int w = getSizeHint().w;
                const float duration = _timeRange.duration().rescaled_to(1.0).value();
                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (seconds > 0.0 && tick > 0)
                {
                    const dtk::Size2I labelMaxSize = _getLabelMaxSize(event.fontSystem);
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = seconds;
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const OTIO_NS::RationalTime time = _timeRange.start_time() +
                            OTIO_NS::RationalTime(t, 1.0).rescaled_to(_timeRange.duration().rate());
                        const dtk::Box2I box(
                            g.min.x +
                            t / duration * w +
                            p.size.border +
                            p.size.margin,
                            p.size.scrollPos.y +
                            g.min.y +
                            p.size.margin,
                            labelMaxSize.w,
                            p.size.fontMetrics.lineHeight);
                        if (time != p.currentTime && dtk::intersects(box, drawRect))
                        {
                            const std::string label = _data->timeUnitsModel->getLabel(time);
                            event.render->drawText(
                                event.fontSystem->getGlyphs(label, p.size.fontInfo),
                                p.size.fontMetrics,
                                box.min,
                                event.style->getColorRole(dtk::ColorRole::TextDisabled));
                        }
                    }
                }
            }
        }

        void TimelineItem::_drawCacheInfo(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            DTK_P();

            const dtk::Box2I& g = getGeometry();

            // Draw the video cache.
            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay ||
                CacheDisplay::VideoOnly == _displayOptions.cacheDisplay)
            {
                dtk::TriMesh2F mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.videoFrames)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const int h = CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay ?
                        p.size.border * 2 :
                        p.size.border * 4;
                    const dtk::Box2I box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin,
                        x1 - x0 + 1,
                        h);
                    if (dtk::intersects(box, drawRect))
                    {
                        mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(dtk::ColorRole::VideoClip));
                }
            }

            // Draw the audio cache.
            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay)
            {
                dtk::TriMesh2F mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.audioFrames)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const dtk::Box2I box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 2,
                        x1 - x0 + 1,
                        p.size.border * 2);
                    if (dtk::intersects(box, drawRect))
                    {
                        mesh.v.push_back(dtk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(dtk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(dtk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(dtk::ColorRole::AudioClip));
                }
            }
        }

        void TimelineItem::_drawCurrentTime(
            const dtk::Box2I& drawRect,
            const dtk::DrawEvent& event)
        {
            DTK_P();

            const dtk::Box2I& g = getGeometry();

            if (!p.currentTime.is_invalid_time())
            {
                const dtk::V2I pos(
                    timeToPos(p.currentTime),
                    p.size.scrollPos.y +
                    g.min.y);

                event.render->drawRect(
                    dtk::Box2I(
                        pos.x,
                        pos.y,
                        p.size.border * 2,
                        g.h()),
                    event.style->getColorRole(dtk::ColorRole::Red));

                const std::string label = _data->timeUnitsModel->getLabel(p.currentTime);
                event.render->drawText(
                    event.fontSystem->getGlyphs(label, p.size.fontInfo),
                    p.size.fontMetrics,
                    dtk::V2I(
                        pos.x + p.size.border * 2 + p.size.margin,
                        pos.y + p.size.margin),
                    event.style->getColorRole(dtk::ColorRole::Text));
            }
        }

        void TimelineItem::_tracksUpdate()
        {
            DTK_P();
            for (const auto& track : p.tracks)
            {
                const bool visible = _isTrackVisible(track.index);
                track.label->setVisible(_displayOptions.trackInfo && visible);
                track.durationLabel->setVisible(_displayOptions.trackInfo && visible);
                for (const auto& item : track.items)
                {
                    item->setVisible(visible);
                }
            }
        }

        void TimelineItem::_textUpdate()
        {
            DTK_P();
            for (const auto& track : p.tracks)
            {
                const OTIO_NS::RationalTime duration = track.timeRange.duration();
                const bool khz =
                    TrackType::Audio == track.type ?
                    (duration.rate() >= 1000.0) :
                    false;
                const OTIO_NS::RationalTime rescaled = duration.rescaled_to(_data->speed);
                const std::string label = dtk::Format("{0}, {1}{2}").
                    arg(_data->timeUnitsModel->getLabel(rescaled)).
                    arg(khz ? (duration.rate() / 1000.0) : duration.rate()).
                    arg(khz ? "kHz" : "FPS");
                track.durationLabel->setText(label);
            }
        }

        TimelineItem::Private::MouseItemData::MouseItemData()
        {}

        TimelineItem::Private::MouseItemData::MouseItemData(
            const std::shared_ptr<IItem>& item,
            int index,
            int track) :
            p(item),
            index(index),
            track(track)
        {
            if (p)
            {
                p->setSelectRole(dtk::ColorRole::Checked);
                geometry = p->getGeometry();
            }
        }

        TimelineItem::Private::MouseItemData::~MouseItemData()
        {
            if (p)
            {
                p->setSelectRole(dtk::ColorRole::None);
                p->setGeometry(geometry);
            }
        }

        std::shared_ptr<IItem> TimelineItem::Private::getAssociated(
            const std::shared_ptr<IItem>& item,
            int& index,
            int& trackIndex) const
        {
            std::shared_ptr<IItem> out;
            if (trackIndex >= 0 && trackIndex < tracks.size() &&
                tracks.size() > 1)
            {
                const OTIO_NS::TimeRange& timeRange = item->getTimeRange();
                if (TrackType::Video == tracks[trackIndex].type &&
                    trackIndex < tracks.size() - 1 &&
                    TrackType::Audio == tracks[trackIndex + 1].type)
                {
                    for (size_t i = 0; i < tracks[trackIndex + 1].items.size(); ++i)
                    {
                        const OTIO_NS::TimeRange& audioTimeRange =
                            tracks[trackIndex + 1].items[i]->getTimeRange();
                        const OTIO_NS::RationalTime audioStartTime =
                            audioTimeRange.start_time().rescaled_to(timeRange.start_time().rate());
                        const OTIO_NS::RationalTime audioDuration =
                            audioTimeRange.duration().rescaled_to(timeRange.duration().rate());
                        if (dtk::fuzzyCompare(
                                audioStartTime.value(),
                                timeRange.start_time().value()) &&
                            dtk::fuzzyCompare(
                                audioDuration.value(),
                                timeRange.duration().value()))
                        {
                            out = tracks[trackIndex + 1].items[i];
                            index = i;
                            trackIndex = trackIndex + 1;
                            break;
                        }
                    }
                }
                else if (TrackType::Audio == tracks[trackIndex].type &&
                    trackIndex > 0 &&
                    TrackType::Video == tracks[trackIndex - 1].type)
                {
                    for (size_t i = 0; i < tracks[trackIndex - 1].items.size(); ++i)
                    {
                        const OTIO_NS::TimeRange& videoTimeRange = 
                            tracks[trackIndex - 1].items[i]->getTimeRange();
                        const OTIO_NS::RationalTime videoStartTime =
                            videoTimeRange.start_time().rescaled_to(timeRange.start_time().rate());
                        const OTIO_NS::RationalTime videoDuration =
                            videoTimeRange.duration().rescaled_to(timeRange.duration().rate());
                        if (dtk::fuzzyCompare(
                                videoStartTime.value(),
                                timeRange.start_time().value()) &&
                            dtk::fuzzyCompare(
                                videoDuration.value(),
                                timeRange.duration().value()))
                        {
                            out = tracks[trackIndex - 1].items[i];
                            index = i;
                            trackIndex = trackIndex - 1;
                            break;
                        }
                    }
                }
            }
            return out;
        }

        std::vector<TimelineItem::Private::MouseItemDropTarget> TimelineItem::Private::getDropTargets(
            const dtk::Box2I& geometry,
            int index,
            int trackIndex)
        {
            std::vector<MouseItemDropTarget> out;
            if (trackIndex >= 0 && trackIndex < tracks.size())
            {
                const auto& track = tracks[trackIndex];
                if (track.type == tracks[trackIndex].type)
                {
                    size_t i = 0;
                    dtk::Box2I g;
                    for (; i < track.items.size(); ++i)
                    {
                        const auto& item = track.items[i];
                        g = item->getGeometry();
                        if (i == index || i == (index + 1))
                        {
                            continue;
                        }
                        MouseItemDropTarget dt;
                        dt.index = i;
                        dt.track = trackIndex;
                        dt.mouse = dtk::Box2I(
                            g.min.x - size.handle,
                            g.min.y,
                            size.handle * 2,
                            g.h());
                        dt.draw = dtk::Box2I(
                            g.min.x - size.border * 2,
                            size.scrollPos.y + geometry.min.y,
                            size.border * 4,
                            geometry.h());
                        out.push_back(dt);
                    }
                    if (!track.items.empty() && index < (track.items.size() - 1))
                    {
                        MouseItemDropTarget dt;
                        dt.index = i;
                        dt.track = trackIndex;
                        dt.mouse = dtk::Box2I(
                            g.max.x - size.handle,
                            g.min.y,
                            size.handle * 2,
                            g.h());
                        dt.draw = dtk::Box2I(
                            g.max.x - size.border * 2,
                            size.scrollPos.y + geometry.min.y,
                            size.border * 4,
                            geometry.h());
                        out.push_back(dt);
                    }
                }
            }
            return out;
        }
    }
}
