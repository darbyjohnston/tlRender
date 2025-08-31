// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItemPrivate.h>

#include <tlTimelineUI/AudioClipItem.h>
#include <tlTimelineUI/GapItem.h>
#include <tlTimelineUI/VideoClipItem.h>

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
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ftk::gl::Window>& window,
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
            FTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true, 1, 0);

            p.player = player;

            p.scrub = ftk::ObservableValue<bool>::create(false);
            p.timeScrub = ftk::ObservableValue<OTIO_NS::RationalTime>::create(time::invalidTime);

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
                        if (-1 == p.firstVideoTrack)
                        {
                            p.firstVideoTrack = track.index;
                        }
                    }
                    else if (OTIO_NS::Track::Kind::audio == otioTrack->kind())
                    {
                        track.type = TrackType::Audio;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Audio Track";
                        }
                        if (-1 == p.firstAudioTrack)
                        {
                            p.firstAudioTrack = track.index;
                        }
                    }
                    track.timeRange = otioTrack->trimmed_range();
                    track.label = ftk::Label::create(
                        context,
                        trackLabel,
                        shared_from_this());
                    track.label->setMarginRole(ftk::SizeRole::MarginInside);
                    track.label->setEnabled(otioTrack->enabled());
                    track.durationLabel = ftk::Label::create(
                        context,
                        shared_from_this());
                    track.durationLabel->setMarginRole(ftk::SizeRole::MarginInside);
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
                                    ftk::ColorRole::VideoGap :
                                    ftk::ColorRole::AudioGap,
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

            p.currentTimeObserver = ftk::ValueObserver<OTIO_NS::RationalTime>::create(
                p.player->observeCurrentTime(),
                [this](const OTIO_NS::RationalTime& value)
                {
                    _p->currentTime = value;
                    _setDrawUpdate();
                });

            p.inOutRangeObserver = ftk::ValueObserver<OTIO_NS::TimeRange>::create(
                p.player->observeInOutRange(),
                [this](const OTIO_NS::TimeRange value)
                {
                    _p->inOutRange = value;
                    _setDrawUpdate();
                });

            p.cacheInfoObserver = ftk::ValueObserver<timeline::PlayerCacheInfo>::create(
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
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<timeline::Player>& player,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<ftk::gl::Window>& window,
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

        void TimelineItem::setStopOnScrub(bool value)
        {
            _p->stopOnScrub = value;
        }

        std::shared_ptr<ftk::IObservableValue<bool> > TimelineItem::observeScrub() const
        {
            return _p->scrub;
        }

        std::shared_ptr<ftk::IObservableValue<OTIO_NS::RationalTime> > TimelineItem::observeTimeScrub() const
        {
            return _p->timeScrub;
        }

        void TimelineItem::setFrameMarkers(const std::vector<int>& value)
        {
            FTK_P();
            if (value == p.frameMarkers)
                return;
            p.frameMarkers = value;
            _setDrawUpdate();
        }

        std::vector<ftk::Box2I> TimelineItem::getTrackGeom() const
        {
            FTK_P();
            std::vector<ftk::Box2I> out;
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
            FTK_P();
            if (changed)
            {
                p.size.displayScale.reset();
                _tracksUpdate();
            }
        }

        void TimelineItem::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            FTK_P();

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

                ftk::Size2I labelSizeHint;
                ftk::Size2I durationSizeHint;
                int trackInfoHeight = 0;
                if (visible && !_displayOptions.minimize)
                {
                    labelSizeHint = track.label->getSizeHint();
                    durationSizeHint = track.durationLabel->getSizeHint();
                    trackInfoHeight = std::max(
                        labelSizeHint.h,
                        durationSizeHint.h);
                }
                track.label->setGeometry(ftk::Box2I(
                    value.min.x,
                    y + trackInfoHeight / 2 - labelSizeHint.h / 2,
                    labelSizeHint.w,
                    labelSizeHint.h));
                track.durationLabel->setGeometry(ftk::Box2I(
                    value.min.x + track.size.w - durationSizeHint.w,
                    y + trackInfoHeight / 2 - durationSizeHint.h / 2,
                    durationSizeHint.w,
                    durationSizeHint.h));

                for (const auto& item : track.items)
                {
                    const OTIO_NS::TimeRange& timeRange = item->getTimeRange();
                    ftk::Size2I sizeHint;
                    if (visible)
                    {
                        sizeHint = item->getSizeHint();
                    }
                    item->setGeometry(ftk::Box2I(
                        value.min.x +
                        timeRange.start_time().rescaled_to(1.0).value() * _scale,
                        y + std::max(labelSizeHint.h, durationSizeHint.h),
                        sizeHint.w,
                        track.clipHeight));
                }

                track.geom = ftk::Box2I(
                    value.min.x,
                    y,
                    track.size.w,
                    visible ? track.size.h : 0);

                if (visible)
                {
                    y += track.size.h;
                }
            }

            if (auto scrollArea = getParentT<ftk::ScrollArea>())
            {
                p.size.scrollArea = ftk::Box2I(
                    scrollArea->getScrollPos(),
                    scrollArea->getGeometry().size());
            }
        }

        void TimelineItem::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            FTK_P();

            if (!p.size.displayScale.has_value() ||
                (p.size.displayScale.has_value() && p.size.displayScale.value() != event.displayScale))
            {
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(ftk::SizeRole::MarginInside, event.displayScale);
                p.size.spacing = event.style->getSizeRole(ftk::SizeRole::SpacingSmall, event.displayScale);
                p.size.border = event.style->getSizeRole(ftk::SizeRole::Border, event.displayScale);
                p.size.handle = event.style->getSizeRole(ftk::SizeRole::Handle, event.displayScale);
                p.size.fontInfo = ftk::FontInfo(
                    _displayOptions.monoFont,
                    _displayOptions.fontSize * event.displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
            }

            int tracksHeight = 0;
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
                        const ftk::Size2I& sizeHint = item->getSizeHint();
                        track.size.h = std::max(track.size.h, sizeHint.h);
                    }
                    track.clipHeight = track.size.h;
                    if (!_displayOptions.minimize)
                    {
                        track.size.h += std::max(
                            track.label->getSizeHint().h,
                            track.durationLabel->getSizeHint().h);
                    }
                    tracksHeight += track.size.h;
                }
            }

            _setSizeHint(ftk::Size2I(
                _timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border +
                tracksHeight));
        }

        void TimelineItem::drawOverlayEvent(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            IItem::drawOverlayEvent(drawRect, event);
            FTK_P();

            const ftk::Box2I& g = getGeometry();

            int y =
                p.size.scrollArea.min.y +
                g.min.y;
            int h =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4;
            event.render->drawRect(
                ftk::Box2I(g.min.x, y, g.w(), h),
                event.style->getColorRole(ftk::ColorRole::Window));

            y = y + h;
            h = p.size.border;
            event.render->drawRect(
                ftk::Box2I(g.min.x, y, g.w(), h),
                event.style->getColorRole(ftk::ColorRole::Border));

            _drawInOutPoints(drawRect, event);
            _drawFrameMarkers(drawRect, event);
            _drawCacheInfo(drawRect, event);
            _drawTimeLabels(drawRect, event);
            _drawTimeTicks(drawRect, event);
            _drawCurrentTime(drawRect, event);
        }

        void TimelineItem::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            FTK_P();
            switch (p.mouseMode)
            {
            case Private::MouseMode::CurrentTime:
            {
                const OTIO_NS::RationalTime time = posToTime(event.pos.x);
                p.timeScrub->setIfChanged(time);
                p.player->seek(time);
                break;
            }
            default: break;
            }
        }

        void TimelineItem::mousePressEvent(ftk::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            FTK_P();
            if (_options.inputEnabled &&
                1 == event.button &&
                0 == event.modifiers)
            {
                takeKeyFocus();

                p.mouseMode = Private::MouseMode::CurrentTime;
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

        void TimelineItem::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            FTK_P();
            p.scrub->setIfChanged(false);
            p.mouseMode = Private::MouseMode::None;
        }

        /*void TimelineItem::keyPressEvent(ftk::KeyEvent& event)
        {
            FTK_P();
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

        void TimelineItem::keyReleaseEvent(ftk::KeyEvent& event)
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

        bool TimelineItem::_isTrackVisible(int index) const
        {
            FTK_P();
            bool out = true;
            if (_displayOptions.minimize)
            {
                out &= index == p.firstVideoTrack || index == p.firstAudioTrack;
            }
            return out;
        }

        ftk::Size2I TimelineItem::_getLabelMaxSize(
            const std::shared_ptr<ftk::FontSystem>& fontSystem) const
        {
            FTK_P();
            const std::string labelMax = _data->timeUnitsModel->getLabel(_timeRange.duration());
            const ftk::Size2I labelMaxSize = fontSystem->getSize(labelMax, p.size.fontInfo);
            return labelMaxSize;
        }

        void TimelineItem::_getTimeTicks(
            const std::shared_ptr<ftk::FontSystem>& fontSystem,
            double& seconds,
            int& tick)
        {
            FTK_P();
            const int w = getSizeHint().w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            const int secondsTick = 1.0 / duration * w;
            const int minutesTick = 60.0 / duration * w;
            const int hoursTick = 3600.0 / duration * w;
            const ftk::Size2I labelMaxSize = _getLabelMaxSize(fontSystem);
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

        void TimelineItem::_drawInOutPoints(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();
            if (!time::compareExact(_p->inOutRange, time::invalidTimeRange) &&
                !time::compareExact(_p->inOutRange, _timeRange))
            {
                const ftk::Box2I& g = getGeometry();
                const ftk::Color4F color(.4F, .5F, .9F);

                const int h = p.size.border * 2;
                switch (_displayOptions.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = timeToPos(_p->inOutRange.start_time());
                    const int x1 = timeToPos(_p->inOutRange.end_time_exclusive());
                    const ftk::Box2I box(
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
                    ftk::Box2I box(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y,
                        x1 - x0 + 1,
                        h);
                    event.render->drawRect(box, color);
                    x0 = timeToPos(_p->inOutRange.end_time_exclusive());
                    x1 = timeToPos(_timeRange.end_time_exclusive());
                    box = ftk::Box2I(
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

        void TimelineItem::_drawFrameMarkers(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();
            const ftk::Box2I& g = getGeometry();
            const double rate = _timeRange.duration().rate();
            const ftk::Color4F color(.6F, .4F, .2F);
            for (const auto& frameMarker : p.frameMarkers)
            {
                const ftk::Box2I g2(
                    timeToPos(OTIO_NS::RationalTime(frameMarker, rate)),
                    p.size.scrollArea.min.y +
                    g.min.y,
                    p.size.border * 2,
                    p.size.margin +
                    p.size.fontMetrics.lineHeight +
                    p.size.margin +
                    p.size.border * 4);
                if (ftk::intersects(g2, drawRect))
                {
                    event.render->drawRect(g2, color);
                }
            }
        }

        void TimelineItem::_drawCacheInfo(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();

            const ftk::Box2I& g = getGeometry();

            // Draw the video cache.
            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay ||
                CacheDisplay::VideoOnly == _displayOptions.cacheDisplay)
            {
                ftk::TriMesh2F mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.video)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const int h = CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay ?
                        p.size.border * 2 :
                        p.size.border * 4;
                    const ftk::Box2I box(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin,
                        x1 - x0 + 1,
                        h);
                    if (ftk::intersects(box, drawRect))
                    {
                        mesh.v.push_back(ftk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(ftk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(ftk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(ftk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(ftk::ColorRole::VideoClip));
                }
            }

            // Draw the audio cache.
            if (CacheDisplay::VideoAndAudio == _displayOptions.cacheDisplay)
            {
                ftk::TriMesh2F mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.audio)
                {
                    const int x0 = timeToPos(t.start_time());
                    const int x1 = timeToPos(t.end_time_exclusive());
                    const ftk::Box2I box(
                        x0,
                        p.size.scrollArea.min.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 2,
                        x1 - x0 + 1,
                        p.size.border * 2);
                    if (ftk::intersects(box, drawRect))
                    {
                        mesh.v.push_back(ftk::V2F(box.min.x, box.min.y));
                        mesh.v.push_back(ftk::V2F(box.max.x + 1, box.min.y));
                        mesh.v.push_back(ftk::V2F(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(ftk::V2F(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(ftk::ColorRole::AudioClip));
                }
            }
        }

        void TimelineItem::_drawTimeLabels(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const ftk::Box2I& g = getGeometry();
                const int w = getSizeHint().w;
                const float duration = _timeRange.duration().rescaled_to(1.0).value();
                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (seconds > 0.0 && tick > 0)
                {
                    const ftk::Size2I labelMaxSize = _getLabelMaxSize(event.fontSystem);
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = seconds;
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const OTIO_NS::RationalTime time = _timeRange.start_time() +
                            OTIO_NS::RationalTime(t, 1.0).rescaled_to(_timeRange.duration().rate());
                        const ftk::Box2I box(
                            g.min.x +
                            t / duration * w +
                            p.size.border +
                            p.size.margin,
                            p.size.scrollArea.min.y +
                            g.min.y +
                            p.size.margin,
                            labelMaxSize.w,
                            p.size.fontMetrics.lineHeight);
                        if (time != p.currentTime && ftk::intersects(box, drawRect))
                        {
                            const std::string label = _data->timeUnitsModel->getLabel(time);
                            event.render->drawText(
                                event.fontSystem->getGlyphs(label, p.size.fontInfo),
                                p.size.fontMetrics,
                                box.min,
                                event.style->getColorRole(ftk::ColorRole::TextDisabled));
                        }
                    }
                }
            }
        }

        void TimelineItem::_drawTimeTicks(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();
            if (_timeRange != time::invalidTimeRange)
            {
                const ftk::Box2I& g = getGeometry();
                const int w = getSizeHint().w;
                const float duration = _timeRange.duration().rescaled_to(1.0).value();
                const int frameTick = 1.0 / _timeRange.duration().value() * w;
                if (duration > 0.0 && frameTick >= p.size.handle)
                {
                    ftk::TriMesh2F mesh;
                    size_t i = 1;
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = 1.0 / _timeRange.duration().rate();
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const ftk::Box2I box(
                            g.min.x +
                            t / duration * w,
                            p.size.scrollArea.min.y +
                            g.min.y +
                            p.size.margin +
                            p.size.fontMetrics.lineHeight,
                            p.size.border,
                            p.size.margin +
                            p.size.border * 4);
                        if (ftk::intersects(box, drawRect))
                        {
                            mesh.v.push_back(ftk::V2F(box.min.x, box.min.y));
                            mesh.v.push_back(ftk::V2F(box.max.x + 1, box.min.y));
                            mesh.v.push_back(ftk::V2F(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(ftk::V2F(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                            mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                            i += 4;
                        }
                    }
                    if (!mesh.v.empty())
                    {
                        event.render->drawMesh(
                            mesh,
                            event.style->getColorRole(ftk::ColorRole::TextDisabled));
                    }
                }

                double seconds = 0;
                int tick = 0;
                _getTimeTicks(event.fontSystem, seconds, tick);
                if (duration > 0.0 && seconds > 0.0 && tick > 0)
                {
                    ftk::TriMesh2F mesh;
                    size_t i = 1;
                    const OTIO_NS::RationalTime t0 = posToTime(g.min.x) - _timeRange.start_time();
                    const OTIO_NS::RationalTime t1 = posToTime(g.max.x) - _timeRange.start_time();
                    const double inc = seconds;
                    const double x0 = static_cast<int>(t0.rescaled_to(1.0).value() / inc) * inc;
                    const double x1 = static_cast<int>(t1.rescaled_to(1.0).value() / inc) * inc;
                    for (double t = x0; t <= x1; t += inc)
                    {
                        const ftk::Box2I box(
                            g.min.x +
                            t / duration * w,
                            p.size.scrollArea.min.y +
                            g.min.y,
                            p.size.border,
                            p.size.margin +
                            p.size.fontMetrics.lineHeight +
                            p.size.margin +
                            p.size.border * 4);
                        if (ftk::intersects(box, drawRect))
                        {
                            mesh.v.push_back(ftk::V2F(box.min.x, box.min.y));
                            mesh.v.push_back(ftk::V2F(box.max.x + 1, box.min.y));
                            mesh.v.push_back(ftk::V2F(box.max.x + 1, box.max.y + 1));
                            mesh.v.push_back(ftk::V2F(box.min.x, box.max.y + 1));
                            mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                            mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                            i += 4;
                        }
                    }
                    if (!mesh.v.empty())
                    {
                        event.render->drawMesh(
                            mesh,
                            event.style->getColorRole(ftk::ColorRole::TextDisabled));
                    }
                }
            }
        }

        void TimelineItem::_drawCurrentTime(
            const ftk::Box2I& drawRect,
            const ftk::DrawEvent& event)
        {
            FTK_P();

            const ftk::Box2I& g = getGeometry();

            if (!p.currentTime.is_invalid_time())
            {
                const ftk::V2I pos(
                    timeToPos(p.currentTime),
                    p.size.scrollArea.min.y +
                    g.min.y);

                event.render->drawRect(
                    ftk::Box2I(
                        pos.x,
                        pos.y,
                        p.size.border * 2,
                        g.h()),
                    event.style->getColorRole(ftk::ColorRole::Red));

                const std::string label = _data->timeUnitsModel->getLabel(p.currentTime);
                ftk::V2I labelPos(
                    pos.x + p.size.border * 2 + p.size.margin,
                    pos.y + p.size.margin);
                const ftk::Size2I labelSize = event.fontSystem->getSize(label, p.size.fontInfo);
                const ftk::Box2I g2(p.size.scrollArea.min + g.min, p.size.scrollArea.size());
                if (labelPos.x + labelSize.w > g2.max.x)
                {
                    const ftk::V2I labelPos2(
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
                    event.style->getColorRole(ftk::ColorRole::Text));
            }
        }

        void TimelineItem::_tracksUpdate()
        {
            FTK_P();
            for (const auto& track : p.tracks)
            {
                const bool visible = _isTrackVisible(track.index);
                track.label->setVisible(!_displayOptions.minimize && visible);
                track.durationLabel->setVisible(!_displayOptions.minimize && visible);
                for (const auto& item : track.items)
                {
                    item->setVisible(visible);
                }
            }
        }

        void TimelineItem::_textUpdate()
        {
            FTK_P();
            for (const auto& track : p.tracks)
            {
                const OTIO_NS::RationalTime duration = track.timeRange.duration();
                const bool khz =
                    TrackType::Audio == track.type ?
                    (duration.rate() >= 1000.0) :
                    false;
                const OTIO_NS::RationalTime rescaled = duration.rescaled_to(_data->speed);
                const std::string label = ftk::Format("{0}, {1}{2}").
                    arg(_data->timeUnitsModel->getLabel(rescaled)).
                    arg(khz ? (duration.rate() / 1000.0) : duration.rate()).
                    arg(khz ? "kHz" : "FPS");
                track.durationLabel->setText(label);
            }
        }
    }
}
