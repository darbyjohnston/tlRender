// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItemPrivate.h>

#include <tlTimelineUI/AudioClipItem.h>
#include <tlTimelineUI/GapItem.h>
#include <tlTimelineUI/VideoClipItem.h>

#include <tlTimeline/Edit.h>
#include <tlTimeline/Util.h>

#include <feather-tk/ui/DrawUtil.h>
#include <feather-tk/ui/ScrollArea.h>
#include <feather-tk/core/Context.h>
#include <feather-tk/core/Format.h>

namespace tl
{
    namespace timelineui
    {
        void TimelineItem::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<feather_tk::gl::Window>& window,
            const std::shared_ptr<IWidget>& parent)
        {
            const OTIO_NS::TimeRange timeRange = player->getTimeRange();
            const OTIO_NS::TimeRange availableRange(
                OTIO_NS::RationalTime(0.0, timeRange.duration().rate()),
                timeRange.duration());
            IItem::_init(
                context,
                "tl::timelineui::TimelineItem",
                timeRange,
                availableRange,
                availableRange,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            FEATHER_TK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true, 0, 0);

            p.player = player;

            p.scrub = feather_tk::ObservableValue<bool>::create(false);
            p.timeScrub = feather_tk::ObservableValue<OTIO_NS::RationalTime>::create(time::invalidTime);

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
                    track.enabledButton = feather_tk::ToolButton::create(
                        context,
                        shared_from_this());
                    track.enabledButton->setIcon("Hidden");
                    track.enabledButton->setCheckedIcon("Visible");
                    track.enabledButton->setCheckedRole(feather_tk::ColorRole::None);
                    track.enabledButton->setCheckable(true);
                    track.enabledButton->setChecked(otioTrack->enabled());
                    track.enabledButton->setCheckedCallback(
                        [this, stackIndex](bool value)
                        {
                            _setTrackEnabled(stackIndex, value);
                        });
                    track.enabledButton->setAcceptsKeyFocus(false);
                    track.enabledButton->setTooltip("Toggle the enabled state");
                    track.label = feather_tk::Label::create(
                        context,
                        trackLabel,
                        shared_from_this());
                    track.label->setMarginRole(feather_tk::SizeRole::MarginInside);
                    track.label->setEnabled(otioTrack->enabled());
                    track.durationLabel = feather_tk::Label::create(
                        context,
                        shared_from_this());
                    track.durationLabel->setMarginRole(feather_tk::SizeRole::MarginInside);
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
                                    feather_tk::ColorRole::VideoGap :
                                    feather_tk::ColorRole::AudioGap,
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

            p.currentTimeObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                p.player->observeCurrentTime(),
                [this](const OTIO_NS::RationalTime& value)
                {
                    _p->currentTime = value;
                    _setDrawUpdate();
                });

            p.inOutRangeObserver = feather_tk::ValueObserver<OTIO_NS::TimeRange>::create(
                p.player->observeInOutRange(),
                [this](const OTIO_NS::TimeRange value)
                {
                    _p->inOutRange = value;
                    _setDrawUpdate();
                });

            p.cacheInfoObserver = feather_tk::ValueObserver<timeline::PlayerCacheInfo>::create(
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
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<feather_tk::gl::Window>& window,
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

        std::shared_ptr<feather_tk::IObservableValue<bool> > TimelineItem::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<feather_tk::IObservableValue<OTIO_NS::RationalTime> > TimelineItem::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        void TimelineItem::setFrameMarkers(const std::vector<int>& value)
        {
            FEATHER_TK_P();
            if (value == p.frameMarkers)
                return;
            p.frameMarkers = value;
            _setDrawUpdate();
        }

        int TimelineItem::getMinimumHeight() const
        {
            return _p->minimumHeight;
        }

        std::vector<feather_tk::Box2I> TimelineItem::getTrackGeom() const
        {
            FEATHER_TK_P();
            std::vector<feather_tk::Box2I> out;
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
            FEATHER_TK_P();
            if (changed)
            {
                p.size.displayScale.reset();
                _tracksUpdate();
            }
        }

        void TimelineItem::setGeometry(const feather_tk::Box2I& value)
        {
            IWidget::setGeometry(value);
            FEATHER_TK_P();

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

                feather_tk::Size2I buttonSizeHint;
                feather_tk::Size2I labelSizeHint;
                feather_tk::Size2I durationSizeHint;
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
                track.enabledButton->setGeometry(feather_tk::Box2I(
                    value.min.x,
                    y + trackInfoHeight / 2 - buttonSizeHint.h / 2,
                    buttonSizeHint.w,
                    buttonSizeHint.h));
                track.label->setGeometry(feather_tk::Box2I(
                    value.min.x + buttonSizeHint.w + p.size.spacing,
                    y + trackInfoHeight / 2 - labelSizeHint.h / 2,
                    labelSizeHint.w,
                    labelSizeHint.h));
                track.durationLabel->setGeometry(feather_tk::Box2I(
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
                    feather_tk::Size2I sizeHint;
                    if (visible)
                    {
                        sizeHint = item->getSizeHint();
                    }
                    item->setGeometry(feather_tk::Box2I(
                        value.min.x +
                        timeRange.start_time().rescaled_to(1.0).value() * _scale,
                        y + std::max(labelSizeHint.h, durationSizeHint.h),
                        sizeHint.w,
                        track.clipHeight));
                }

                track.geom = feather_tk::Box2I(
                    value.min.x,
                    y,
                    track.size.w,
                    visible ? track.size.h : 0);

                if (visible)
                {
                    y += track.size.h;
                }
            }

            if (auto scrollArea = getParentT<feather_tk::ScrollArea>())
            {
                p.size.scrollArea = feather_tk::Box2I(
                    scrollArea->getScrollPos(),
                    scrollArea->getGeometry().size());
            }
        }

        void TimelineItem::sizeHintEvent(const feather_tk::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            FEATHER_TK_P();

            if (!p.size.displayScale.has_value() ||
                (p.size.displayScale.has_value() && p.size.displayScale.value() != event.displayScale))
            {
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(feather_tk::SizeRole::MarginInside, event.displayScale);
                p.size.spacing = event.style->getSizeRole(feather_tk::SizeRole::SpacingSmall, event.displayScale);
                p.size.border = event.style->getSizeRole(feather_tk::SizeRole::Border, event.displayScale);
                p.size.handle = event.style->getSizeRole(feather_tk::SizeRole::Handle, event.displayScale);
                p.size.fontInfo = feather_tk::FontInfo(
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
                        const feather_tk::Size2I& sizeHint = item->getSizeHint();
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

            _setSizeHint(feather_tk::Size2I(
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
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            IItem::drawOverlayEvent(drawRect, event);
            FEATHER_TK_P();

            const feather_tk::Box2I& g = getGeometry();

            int y =
                p.size.scrollArea.min.y +
                g.min.y;
            int h =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4;
            event.render->drawRect(
                feather_tk::Box2I(g.min.x, y, g.w(), h),
                event.style->getColorRole(feather_tk::ColorRole::Window));

            y = y + h;
            h = p.size.border;
            event.render->drawRect(
                feather_tk::Box2I(g.min.x, y, g.w(), h),
                event.style->getColorRole(feather_tk::ColorRole::Border));

            _drawInOutPoints(drawRect, event);
            _drawFrameMarkers(drawRect, event);
            _drawTimeLabels(drawRect, event);
            _drawCacheInfo(drawRect, event);
            _drawTimeTicks(drawRect, event);
            _drawCurrentTime(drawRect, event);

            if (p.mouse.currentDropTarget >= 0 &&
                p.mouse.currentDropTarget < p.mouse.dropTargets.size())
            {
                const auto& dt = p.mouse.dropTargets[p.mouse.currentDropTarget];
                event.render->drawRect(
                    dt.draw,
                    event.style->getColorRole(feather_tk::ColorRole::Green));
            }
        }

        void TimelineItem::mouseMoveEvent(feather_tk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            FEATHER_TK_P();
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
                        const feather_tk::Box2I& g = item->geometry;
                        item->p->setGeometry(feather_tk::Box2I(
                            g.min + _getMousePos() - _getMousePressPos(),
                            g.size()));
                    }
                    
                    int dropTarget = -1;
                    for (size_t i = 0; i < p.mouse.dropTargets.size(); ++i)
                    {
                        if (feather_tk::contains(p.mouse.dropTargets[i].mouse, event.pos))
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
                                feather_tk::ColorRole::Green :
                                feather_tk::ColorRole::Checked);
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

        void TimelineItem::mousePressEvent(feather_tk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            FEATHER_TK_P();
            if (_options.inputEnabled &&
                0 == event.button &&
                0 == event.modifiers)
            {
                takeKeyFocus();

                p.mouse.mode = Private::MouseMode::None;

                const feather_tk::Box2I& g = getGeometry();
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
                                if (feather_tk::contains(item->getGeometry(), event.pos))
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
                        p.player->stop();
                    }
                    const OTIO_NS::RationalTime time = posToTime(event.pos.x);
                    p.scrub->setIfChanged(true);
                    p.timeScrub->setIfChanged(time);
                    p.player->seek(time);
                }
            }
        }

        void TimelineItem::mouseReleaseEvent(feather_tk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            FEATHER_TK_P();
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

        /*void TimelineItem::keyPressEvent(feather_tk::KeyEvent& event)
        {
            FEATHER_TK_P();
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

        void TimelineItem::keyReleaseEvent(feather_tk::KeyEvent& event)
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
            FEATHER_TK_P();
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
            FEATHER_TK_P();
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
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();
            if (!time::compareExact(_p->inOutRange, time::invalidTimeRange) &&
                !time::compareExact(_p->inOutRange, _timeRange))
            {
                const feather_tk::Box2I& g = getGeometry();
                const feather_tk::Color4F color(.4F, .5F, .9F);

                const int h = p.size.border * 2;
                switch (_displayOptions.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = timeToPos(_p->inOutRange.start_time());
                    const int x1 = timeToPos(_p->inOutRange.end_time_exclusive());
                    const feather_tk::Box2I box(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y,
                        x1 - x0 + 1,
                        h);
                    event.render->drawRect(box, color);
                    break;
                }
                case InOutDisplay::OutsideRange:
                {
                    int x0 = timeToPos(_timeRange.start_time());
                    int x1 = timeToPos(_p->inOutRange.start_time());
                    feather_tk::Box2I box(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y,
                        x1 - x0 + 1,
                        h);
                    event.render->drawRect(box, color);
                    x0 = timeToPos(_p->inOutRange.end_time_exclusive());
                    x1 = timeToPos(_timeRange.end_time_exclusive());
                    box = feather_tk::Box2I(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y,
                        x1 - x0 + 1,
                        h);
                    event.render->drawRect(box, color);
                    break;
                }
                default: break;
                }
            }
        }

        feather_tk::Size2I TimelineItem::_getLabelMaxSize(
            const std::shared_ptr<feather_tk::FontSystem>& fontSystem) const
        {
            FEATHER_TK_P();
            const std::string labelMax = _data->timeUnitsModel->getLabel(_timeRange.duration());
            const feather_tk::Size2I labelMaxSize = fontSystem->getSize(labelMax, p.size.fontInfo);
            return labelMaxSize;
        }

        void TimelineItem::_getTimeTicks(
            const std::shared_ptr<feather_tk::FontSystem>& fontSystem,
            double& seconds,
            int& tick)
        {
            FEATHER_TK_P();
            const int w = getSizeHint().w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            const int secondsTick = 1.0 / duration * w;
            const int minutesTick = 60.0 / duration * w;
            const int hoursTick = 3600.0 / duration * w;
            const feather_tk::Size2I labelMaxSize = _getLabelMaxSize(fontSystem);
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

        void TimelineItem::_drawFrameMarkers(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();
            const feather_tk::Box2I& g = getGeometry();
            const double rate = _timeRange.duration().rate();
            const feather_tk::Color4F color(.6F, .4F, .2F);
            for (const auto& frameMarker : p.frameMarkers)
            {
                const feather_tk::Box2I g2(
                    timeToPos(OTIO_NS::RationalTime(frameMarker, rate)),
                    p.size.scrollArea.min.y +
                    g.min.y,
                    p.size.border * 2,
                    p.size.margin +
                    p.size.fontMetrics.lineHeight +
                    p.size.margin +
                    p.size.border * 4);
                if (feather_tk::intersects(g2, drawRect))
                {
                    event.render->drawRect(g2, color);
                }
            }
        }

        void TimelineItem::_drawTimeLabels(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const feather_tk::Box2I& g = getGeometry();
                const int w = getSizeHint().w;
                const float duration = _timeRange.duration().rescaled_to(1.0).value();
                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (seconds > 0.0 && tick > 0)
                {
                    const feather_tk::Size2I labelMaxSize = _getLabelMaxSize(event.fontSystem);
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = seconds;
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const OTIO_NS::RationalTime time = _timeRange.start_time() +
                            OTIO_NS::RationalTime(t, 1.0).rescaled_to(_timeRange.duration().rate());
                        const feather_tk::Box2I box(
                            g.min.x +
                            t / duration * w +
                            p.size.border +
                            p.size.margin,
                            p.size.scrollArea.min.y +
                            g.min.y +
                            p.size.margin,
                            labelMaxSize.w,
                            p.size.fontMetrics.lineHeight);
                        if (time != p.currentTime && feather_tk::intersects(box, drawRect))
                        {
                            const std::string label = _data->timeUnitsModel->getLabel(time);
                            event.render->drawText(
                                event.fontSystem->getGlyphs(label, p.size.fontInfo),
                                p.size.fontMetrics,
                                box.min,
                                event.style->getColorRole(feather_tk::ColorRole::TextDisabled));
                        }
                    }
                }
            }
        }

        void TimelineItem::_drawCacheInfo(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();

            const feather_tk::Box2I& g = getGeometry();

            // Draw the video cache.
            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay ||
                CacheDisplay::VideoOnly == _displayOptions.cacheDisplay)
            {
                feather_tk::TriMesh2F mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.video)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const int h = CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay ?
                        p.size.border * 2 :
                        p.size.border * 4;
                    const feather_tk::Box2I box(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin,
                        x1 - x0 + 1,
                        h);
                    if (feather_tk::intersects(box, drawRect))
                    {
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(feather_tk::ColorRole::VideoClip));
                }
            }

            // Draw the audio cache.
            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay)
            {
                feather_tk::TriMesh2F mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.audio)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const feather_tk::Box2I box(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 2,
                        x1 - x0 + 1,
                        p.size.border * 2);
                    if (feather_tk::intersects(box, drawRect))
                    {
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(feather_tk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(feather_tk::ColorRole::AudioClip));
                }
            }
        }

        void TimelineItem::_drawTimeTicks(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const feather_tk::Box2I& g = getGeometry();
                const int w = getSizeHint().w;
                const float duration = _timeRange.duration().rescaled_to(1.0).value();
                const int frameTick = 1.0 / _timeRange.duration().value() * w;
                if (duration > 0.0 && frameTick >= p.size.handle)
                {
                    feather_tk::TriMesh2F mesh;
                    size_t i = 1;
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = 1.0 / _timeRange.duration().rate();
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const feather_tk::Box2I box(
                            g.min.x +
                            t / duration * w,
                            p.size.scrollArea.min.y +
                            g.min.y +
                            p.size.margin +
                            p.size.fontMetrics.lineHeight,
                            p.size.border,
                            p.size.margin +
                            p.size.border * 4);
                        if (feather_tk::intersects(box, drawRect))
                        {
                            mesh.v.push_back(feather_tk::V2F(box.min.x, box.min.y));
                            mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.min.y));
                            mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(feather_tk::V2F(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                            mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                            i += 4;
                        }
                    }
                    if (!mesh.v.empty())
                    {
                        event.render->drawMesh(
                            mesh,
                            event.style->getColorRole(feather_tk::ColorRole::TextDisabled));
                    }
                }

                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (duration > 0.0 && seconds > 0.0 && tick > 0)
                {
                    feather_tk::TriMesh2F mesh;
                    size_t i = 1;
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = seconds;
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const feather_tk::Box2I box(
                            g.min.x +
                            t / duration * w,
                            p.size.scrollArea.min.y +
                            g.min.y,
                            p.size.border,
                            p.size.margin +
                            p.size.fontMetrics.lineHeight +
                            p.size.margin +
                            p.size.border * 4);
                        if (feather_tk::intersects(box, drawRect))
                        {
                            mesh.v.push_back(feather_tk::V2F(box.min.x, box.min.y));
                            mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.min.y));
                            mesh.v.push_back(feather_tk::V2F(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(feather_tk::V2F(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                            mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                            i += 4;
                        }
                    }
                    if (!mesh.v.empty())
                    {
                        event.render->drawMesh(
                            mesh,
                            event.style->getColorRole(feather_tk::ColorRole::TextDisabled));
                    }
                }
            }
        }

        void TimelineItem::_drawCurrentTime(
            const feather_tk::Box2I& drawRect,
            const feather_tk::DrawEvent& event)
        {
            FEATHER_TK_P();

            const feather_tk::Box2I& g = getGeometry();

            if (!p.currentTime.is_invalid_time())
            {
                const feather_tk::V2I pos(
                    timeToPos(p.currentTime),
                    p.size.scrollArea.min.y +
                    g.min.y);

                event.render->drawRect(
                    feather_tk::Box2I(
                        pos.x,
                        pos.y,
                        p.size.border * 2,
                        g.h()),
                    event.style->getColorRole(feather_tk::ColorRole::Red));

                const std::string label = _data->timeUnitsModel->getLabel(p.currentTime);
                feather_tk::V2I labelPos(
                    pos.x + p.size.border * 2 + p.size.margin,
                    pos.y + p.size.margin);
                const feather_tk::Size2I labelSize = event.fontSystem->getSize(label, p.size.fontInfo);
                const feather_tk::Box2I g2(p.size.scrollArea.min + g.min, p.size.scrollArea.size());
                if (labelPos.x + labelSize.w > g2.max.x)
                {
                    const feather_tk::V2I labelPos2(
                        pos.x - p.size.border * 2 - p.size.margin - labelSize.w,
                        pos.y + p.size.margin);
                    if (labelPos2.x > g2.min.x)
                    {
                        labelPos = labelPos2;
                    }
                }
                event.render->drawText(
                    event.fontSystem->getGlyphs(label, p.size.fontInfo),
                    p.size.fontMetrics,
                    labelPos,
                    event.style->getColorRole(feather_tk::ColorRole::Text));
            }
        }

        void TimelineItem::_tracksUpdate()
        {
            FEATHER_TK_P();
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
            FEATHER_TK_P();
            for (const auto& track : p.tracks)
            {
                const OTIO_NS::RationalTime duration = track.timeRange.duration();
                const bool khz =
                    TrackType::Audio == track.type ?
                    (duration.rate() >= 1000.0) :
                    false;
                const OTIO_NS::RationalTime rescaled = duration.rescaled_to(_data->speed);
                const std::string label = feather_tk::Format("{0}, {1}{2}").
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
                p->setSelectRole(feather_tk::ColorRole::Checked);
                geometry = p->getGeometry();
            }
        }

        TimelineItem::Private::MouseItemData::~MouseItemData()
        {
            if (p)
            {
                p->setSelectRole(feather_tk::ColorRole::None);
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
                        if (feather_tk::fuzzyCompare(
                                audioStartTime.value(),
                                timeRange.start_time().value()) &&
                            feather_tk::fuzzyCompare(
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
                        if (feather_tk::fuzzyCompare(
                                videoStartTime.value(),
                                timeRange.start_time().value()) &&
                            feather_tk::fuzzyCompare(
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
            const feather_tk::Box2I& geometry,
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
                    feather_tk::Box2I g;
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
                        dt.mouse = feather_tk::Box2I(
                            g.min.x - size.handle,
                            g.min.y,
                            size.handle * 2,
                            g.h());
                        dt.draw = feather_tk::Box2I(
                            g.min.x - size.border * 2,
                            size.scrollArea.min.y + geometry.min.y,
                            size.border * 4,
                            geometry.h());
                        out.push_back(dt);
                    }
                    if (!track.items.empty() && index < (track.items.size() - 1))
                    {
                        MouseItemDropTarget dt;
                        dt.index = i;
                        dt.track = trackIndex;
                        dt.mouse = feather_tk::Box2I(
                            g.max.x - size.handle,
                            g.min.y,
                            size.handle * 2,
                            g.h());
                        dt.draw = feather_tk::Box2I(
                            g.max.x - size.border * 2,
                            size.scrollArea.min.y + geometry.min.y,
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
