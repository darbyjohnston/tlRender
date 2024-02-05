// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItem.h>

#include <tlTimelineUI/AudioClipItem.h>
#include <tlTimelineUI/GapItem.h>
#include <tlTimelineUI/VideoClipItem.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/Label.h>
#include <tlUI/ScrollArea.h>

#include <tlTimeline/Edit.h>
#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineItem::Private
        {
            std::shared_ptr<timeline::Player> player;
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            timeline::PlayerCacheInfo cacheInfo;
            bool editable = false;
            bool stopOnScrub = true;
            std::shared_ptr<ui::ThumbnailGenerator> thumbnailGenerator;

            struct Track
            {
                TrackType type = TrackType::None;
                otime::TimeRange timeRange;
                std::shared_ptr<ui::Label> label;
                std::shared_ptr<ui::Label> durationLabel;
                std::vector<std::shared_ptr<IItem> > items;
                math::Size2i size;
                int clipHeight = 0;
            };
            std::vector<Track> tracks;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int border = 0;
                int handle = 0;
                image::FontInfo fontInfo = image::FontInfo("", 0);
                image::FontMetrics fontMetrics;

                math::Vector2i scrollPos;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<math::Box2i> dropTargets;
            };
            DrawData draw;

            enum class MouseMode
            {
                None,
                CurrentTime,
                Item
            };
            struct MouseItemData
            {
                MouseItemData();
                MouseItemData(
                    const std::shared_ptr<IItem>&,
                    int index,
                    int track);

                ~MouseItemData();

                std::shared_ptr<IItem> p;
                int index = -1;
                int track = -1;
                math::Box2i geometry;
            };
            struct MouseItemDropTarget
            {
                int index = -1;
                int track = -1;
                math::Box2i mouse;
                math::Box2i draw;
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                std::vector<std::shared_ptr<MouseItemData> > items;
                std::vector<MouseItemDropTarget> dropTargets;
                int currentDropTarget = -1;
            };
            MouseData mouse;

            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> > inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;

            std::shared_ptr<IItem> getAssociated(
                const std::shared_ptr<IItem>&,
                int& index,
                int& trackIndex) const;

            std::vector<MouseItemDropTarget> getDropTargets(
                const math::Box2i& geometry,
                int index,
                int track);
        };

        void TimelineItem::_init(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<gl::GLFWWindow>& window,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const otime::TimeRange timeRange = player->getTimeRange();
            const otime::TimeRange trimmedRange(
                otime::RationalTime(0.0, timeRange.duration().rate()),
                timeRange.duration());
            IItem::_init(
                "tl::timelineui::TimelineItem",
                timeRange,
                trimmedRange,
                scale,
                options,
                itemData,
                context,
                parent);
            TLRENDER_P();

            _setMouseHover(true);
            _setMousePress(true, 0, 0);

            p.player = player;
            
            p.thumbnailGenerator = ui::ThumbnailGenerator::create(context, window);

            const auto otioTimeline = p.player->getTimeline()->getTimeline();
            int trackIndex = 0;
            for (const auto& child : otioTimeline->tracks()->children())
            {
                if (auto otioTrack = otio::dynamic_retainer_cast<otio::Track>(child))
                {
                    Private::Track track;
                    std::string trackLabel = otioTrack->name();
                    if (otio::Track::Kind::video == otioTrack->kind())
                    {
                        track.type = TrackType::Video;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Video Track";
                        }
                    }
                    else if (otio::Track::Kind::audio == otioTrack->kind())
                    {
                        track.type = TrackType::Audio;
                        if (trackLabel.empty())
                        {
                            trackLabel = "Audio Track";
                        }
                    }
                    track.timeRange = otioTrack->trimmed_range();
                    track.label = ui::Label::create(
                        trackLabel,
                        context,
                        shared_from_this());
                    track.label->setMarginRole(ui::SizeRole::MarginInside);
                    track.durationLabel = ui::Label::create(
                        context,
                        shared_from_this());
                    track.durationLabel->setMarginRole(ui::SizeRole::MarginInside);

                    for (const auto& child : otioTrack->children())
                    {
                        if (auto clip = otio::dynamic_retainer_cast<otio::Clip>(child))
                        {
                            switch (track.type)
                            {
                            case TrackType::Video:
                                track.items.push_back(VideoClipItem::create(
                                    clip,
                                    scale,
                                    options,
                                    itemData,
                                    p.thumbnailGenerator,
                                    context,
                                    shared_from_this()));
                                break;
                            case TrackType::Audio:
                                track.items.push_back(AudioClipItem::create(
                                    clip,
                                    scale,
                                    options,
                                    itemData,
                                    p.thumbnailGenerator,
                                    context,
                                    shared_from_this()));
                                break;
                            default: break;
                            }
                        }
                        else if (auto gap = otio::dynamic_retainer_cast<otio::Gap>(child))
                        {
                            track.items.push_back(GapItem::create(
                                TrackType::Video == track.type ?
                                ui::ColorRole::VideoGap :
                                ui::ColorRole::AudioGap,
                                gap,
                                scale,
                                options,
                                itemData,
                                context,
                                shared_from_this()));
                        }
                    }

                    p.tracks.push_back(track);
                }
            }

            _textUpdate();

            p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                p.player->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                {
                    _p->currentTime = value;
                    _updates |= ui::Update::Draw;
                });

            p.inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                p.player->observeInOutRange(),
                [this](const otime::TimeRange value)
                {
                    _p->inOutRange = value;
                    _updates |= ui::Update::Draw;
                });

            p.cacheInfoObserver = observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                p.player->observeCacheInfo(),
                [this](const timeline::PlayerCacheInfo& value)
                {
                    _p->cacheInfo = value;
                    _updates |= ui::Update::Draw;
                });
        }

        TimelineItem::TimelineItem() :
            _p(new Private)
        {}

        TimelineItem::~TimelineItem()
        {}

        std::shared_ptr<TimelineItem> TimelineItem::create(
            const std::shared_ptr<timeline::Player>& player,
            const otio::SerializableObject::Retainer<otio::Stack>& stack,
            double scale,
            const ItemOptions& options,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<gl::GLFWWindow>& window,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
            out->_init(player, stack, scale, options, itemData, window, context, parent);
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

        void TimelineItem::setOptions(const ItemOptions& value)
        {
            const bool changed = value != _options;
            IItem::setOptions(value);
            TLRENDER_P();
            if (changed)
            {
                p.size.sizeInit = true;
            }
        }

        void TimelineItem::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();

            const math::Box2i& g = _geometry;
            float y =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border +
                g.min.y;
            for (const auto& track : p.tracks)
            {
                const math::Size2i& labelSizeHint = track.label->getSizeHint();
                track.label->setGeometry(math::Box2i(
                    g.min.x,
                    y,
                    labelSizeHint.w,
                    labelSizeHint.h));
                const math::Size2i& durationSizeHint = track.durationLabel->getSizeHint();
                track.durationLabel->setGeometry(math::Box2i(
                    g.min.x + track.size.w - durationSizeHint.w,
                    y,
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
                    const otime::TimeRange& timeRange = item->getTimeRange();
                    const math::Size2i& sizeHint = item->getSizeHint();
                    item->setGeometry(math::Box2i(
                        _geometry.min.x +
                        timeRange.start_time().rescaled_to(1.0).value() * _scale,
                        y + std::max(labelSizeHint.h, durationSizeHint.h),
                        sizeHint.w,
                        track.clipHeight));
                }

                y += track.size.h;
            }

            if (auto scrollArea = getParentT<ui::ScrollArea>())
            {
                p.size.scrollPos = scrollArea->getScrollPos();
            }
        }

        void TimelineItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, _displayScale);
                p.size.border = event.style->getSizeRole(ui::SizeRole::Border, _displayScale);
                p.size.handle = event.style->getSizeRole(ui::SizeRole::Handle, _displayScale);
                p.size.fontInfo = image::FontInfo(
                    _options.monoFont,
                    _options.fontSize * _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
            }
            p.size.sizeInit = false;

            int tracksHeight = 0;
            for (auto& track : p.tracks)
            {
                track.size.w = track.timeRange.duration().rescaled_to(1.0).value() * _scale;
                track.size.h = 0;
                for (const auto& item : track.items)
                {
                    const math::Size2i& sizeHint = item->getSizeHint();
                    track.size.h = std::max(track.size.h, sizeHint.h);
                }
                track.clipHeight = track.size.h;
                track.size.h += std::max(
                    track.label->getSizeHint().h,
                    track.durationLabel->getSizeHint().h);
                tracksHeight += track.size.h;
            }

            _sizeHint = math::Size2i(
                _timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border +
                tracksHeight);
        }

        void TimelineItem::drawOverlayEvent(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawOverlayEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            int y =
                p.size.scrollPos.y +
                g.min.y;
            int h =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4;
            event.render->drawRect(
                math::Box2i(g.min.x, y, g.w(), h),
                event.style->getColorRole(ui::ColorRole::Window));

            y = y + h;
            h = p.size.border;
            event.render->drawRect(
                math::Box2i(g.min.x, y, g.w(), h),
                event.style->getColorRole(ui::ColorRole::Border));

            _drawInOutPoints(drawRect, event);
            _drawTimeTicks(drawRect, event);
            _drawCacheInfo(drawRect, event);
            _drawCurrentTime(drawRect, event);

            if (p.mouse.currentDropTarget >= 0 &&
                p.mouse.currentDropTarget < p.mouse.dropTargets.size())
            {
                const auto& dt = p.mouse.dropTargets[p.mouse.currentDropTarget];
                event.render->drawRect(
                    dt.draw,
                    event.style->getColorRole(ui::ColorRole::Green));
            }
        }

        void TimelineItem::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            IWidget::mouseMoveEvent(event);
            TLRENDER_P();
            switch (p.mouse.mode)
            {
            case Private::MouseMode::CurrentTime:
                p.player->seek(_posToTime(event.pos.x));
                break;
            case Private::MouseMode::Item:
            {
                if (!p.mouse.items.empty())
                {
                    for (const auto& item : p.mouse.items)
                    {
                        const math::Box2i& g = item->geometry;
                        item->p->setGeometry(math::Box2i(
                            g.min + _mouse.pos - _mouse.pressPos,
                            g.getSize()));
                    }
                    
                    int dropTarget = -1;
                    for (size_t i = 0; i < p.mouse.dropTargets.size(); ++i)
                    {
                        if (p.mouse.dropTargets[i].mouse.contains(event.pos))
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
                                ui::ColorRole::Green :
                                ui::ColorRole::Checked);
                        }
                        p.mouse.currentDropTarget = dropTarget;
                        _updates |= ui::Update::Draw;
                    }
                }
                break;
            }
            default: break;
            }
        }

        void TimelineItem::mousePressEvent(ui::MouseClickEvent& event)
        {
            IWidget::mousePressEvent(event);
            TLRENDER_P();
            if (0 == event.button && 0 == event.modifiers)
            {
                takeKeyFocus();

                p.mouse.mode = Private::MouseMode::None;

                const math::Box2i& g = _geometry;
                if (p.editable)
                {
                    for (int i = 0; i < p.tracks.size(); ++i)
                    {
                        const auto& items = p.tracks[i].items;
                        for (int j = 0; j < items.size(); ++j)
                        {
                            const auto& item = items[j];
                            if (item->getGeometry().contains(event.pos))
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
                    p.player->seek(_posToTime(event.pos.x));
                }
            }
        }

        void TimelineItem::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            IWidget::mouseReleaseEvent(event);
            TLRENDER_P();
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
                _updates |= ui::Update::Draw;
            }
            p.mouse.currentDropTarget = -1;
        }

        /*void TimelineItem::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && 0 == event.modifiers)
            {
                switch (event.key)
                {
                default: break;
                }
            }
        }

        void TimelineItem::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
        }*/

        void TimelineItem::_timeUnitsUpdate()
        {
            IItem::_timeUnitsUpdate();
            _textUpdate();
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void TimelineItem::_releaseMouse()
        {
            TLRENDER_P();
            IWidget::_releaseMouse();
            p.mouse.items.clear();
        }

        void TimelineItem::_drawInOutPoints(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();
            if (!time::compareExact(_p->inOutRange, time::invalidTimeRange) &&
                !time::compareExact(_p->inOutRange, _timeRange))
            {
                const math::Box2i& g = _geometry;

                switch (_options.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = _timeToPos(_p->inOutRange.start_time());
                    const int x1 = _timeToPos(_p->inOutRange.end_time_exclusive());
                    const math::Box2i box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(
                        box,
                        event.style->getColorRole(ui::ColorRole::InOut));
                    break;
                }
                case InOutDisplay::OutsideRange:
                {
                    int x0 = _timeToPos(_timeRange.start_time());
                    int x1 = _timeToPos(_p->inOutRange.start_time());
                    math::Box2i box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(
                        box,
                        event.style->getColorRole(ui::ColorRole::InOut));
                    x0 = _timeToPos(_p->inOutRange.end_time_exclusive());
                    x1 = _timeToPos(_timeRange.end_time_exclusive());
                    box = math::Box2i(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(
                        box,
                        event.style->getColorRole(ui::ColorRole::InOut));
                    break;
                }
                default: break;
                }
            }
        }

        void TimelineItem::_drawTimeTicks(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            const std::string labelMax = _data->timeUnitsModel->getLabel(_timeRange.duration());
            const math::Size2i labelMaxSize = event.fontSystem->getSize(labelMax, p.size.fontInfo);
            const int distanceMin = p.size.border + p.size.margin + labelMaxSize.w;

            const int w = _sizeHint.w;
            const float duration = _timeRange.duration().rescaled_to(1.0).value();
            const int frameTick = 1.0 / _timeRange.duration().value() * w;
            if (frameTick >= p.size.handle)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (double t = 0.0; t < duration; t += 1.0 / _timeRange.duration().rate())
                {
                    const math::Box2i box(
                        g.min.x +
                        t / duration * w,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight,
                        p.size.border,
                        p.size.margin +
                        p.size.border * 4);
                    if (box.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(math::Vector2f(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::Button));
                }
            }

            const int secondsTick = 1.0 / duration * w;
            const int minutesTick = 60.0 / duration * w;
            const int hoursTick = 3600.0 / duration * w;
            double seconds = 0;
            int tick = 0;
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
            if (seconds > 0.0 && tick > 0)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (double t = 0.0; t < duration; t += seconds)
                {
                    const math::Box2i box(
                        g.min.x +
                        t / duration * w,
                        p.size.scrollPos.y +
                        g.min.y,
                        p.size.border,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 4);
                    if (box.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(math::Vector2f(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::Button));
                }

                const otime::RationalTime& currentTime = p.player->observeCurrentTime()->get();
                for (double t = 0.0; t < duration; t += seconds)
                {
                    const otime::RationalTime time = _timeRange.start_time() +
                        otime::RationalTime(t, 1.0).rescaled_to(_timeRange.duration().rate());
                    const math::Box2i box(
                        g.min.x +
                        t / duration * w +
                        p.size.border +
                        p.size.margin,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin,
                        labelMaxSize.w,
                        p.size.fontMetrics.lineHeight);
                    if (time != currentTime && box.intersects(drawRect))
                    {
                        const std::string label = _data->timeUnitsModel->getLabel(time);
                        event.render->drawText(
                            event.fontSystem->getGlyphs(label, p.size.fontInfo),
                            math::Vector2i(
                                box.min.x,
                                box.min.y +
                                p.size.fontMetrics.ascender),
                            event.style->getColorRole(ui::ColorRole::TextDisabled));
                    }
                }
            }
        }

        void TimelineItem::_drawCacheInfo(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            if (CacheDisplay::VideoAndAudio == _options.cacheDisplay ||
                CacheDisplay::VideoOnly == _options.cacheDisplay)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.videoFrames)
                {
                    const int x0 = _timeToPos(t.start_time());
                    const int x1 = _timeToPos(t.end_time_exclusive());
                    const int h = CacheDisplay::VideoAndAudio == _options.cacheDisplay ?
                        p.size.border * 2 :
                        p.size.border * 4;
                    const math::Box2i box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin,
                        x1 - x0 + 1,
                        h);
                    if (box.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(math::Vector2f(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::VideoCache));
                }
            }

            if (CacheDisplay::VideoAndAudio == _options.cacheDisplay)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (const auto& t : p.cacheInfo.audioFrames)
                {
                    const int x0 = _timeToPos(t.start_time());
                    const int x1 = _timeToPos(t.end_time_exclusive());
                    const math::Box2i box(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 2,
                        x1 - x0 + 1,
                        p.size.border * 2);
                    if (box.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(box.min.x, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.min.y));
                        mesh.v.push_back(math::Vector2f(box.max.x + 1, box.max.y + 1));
                        mesh.v.push_back(math::Vector2f(box.min.x, box.max.y + 1));
                        mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                        mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                        i += 4;
                    }
                }
                if (!mesh.v.empty())
                {
                    event.render->drawMesh(
                        mesh,
                        math::Vector2i(),
                        event.style->getColorRole(ui::ColorRole::AudioCache));
                }
            }
        }

        void TimelineItem::_drawCurrentTime(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            const otime::RationalTime& currentTime = p.player->observeCurrentTime()->get();
            if (!time::compareExact(currentTime, time::invalidTime))
            {
                const math::Vector2i pos(
                    _timeToPos(currentTime),
                    p.size.scrollPos.y +
                    g.min.y);

                event.render->drawRect(
                    math::Box2i(
                        pos.x,
                        pos.y,
                        p.size.border * 2,
                        g.h()),
                    event.style->getColorRole(ui::ColorRole::Red));

                const std::string label = _data->timeUnitsModel->getLabel(currentTime);
                event.render->drawText(
                    event.fontSystem->getGlyphs(label, p.size.fontInfo),
                    math::Vector2i(
                        pos.x + p.size.border * 2 + p.size.margin,
                        pos.y +
                        p.size.margin +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        void TimelineItem::_textUpdate()
        {
            TLRENDER_P();
            for (const auto& track : p.tracks)
            {
                const otime::RationalTime duration = track.timeRange.duration();
                const bool khz =
                    TrackType::Audio == track.type ?
                    (duration.rate() >= 1000.0) :
                    false;
                const otime::RationalTime rescaled = duration.rescaled_to(_data->speed);
                const std::string label = string::Format("{0}, {1}{2}").
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
                p->setSelectRole(ui::ColorRole::Checked);
                geometry = p->getGeometry();
            }
        }

        TimelineItem::Private::MouseItemData::~MouseItemData()
        {
            if (p)
            {
                p->setSelectRole(ui::ColorRole::None);
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
                const otime::TimeRange& timeRange = item->getTimeRange();
                if (TrackType::Video == tracks[trackIndex].type &&
                    trackIndex < tracks.size() - 1 &&
                    TrackType::Audio == tracks[trackIndex + 1].type)
                {
                    for (size_t i = 0; i < tracks[trackIndex + 1].items.size(); ++i)
                    {
                        const otime::TimeRange& audioTimeRange =
                            tracks[trackIndex + 1].items[i]->getTimeRange();
                        const otime::RationalTime audioStartTime =
                            audioTimeRange.start_time().rescaled_to(timeRange.start_time().rate());
                        const otime::RationalTime audioDuration =
                            audioTimeRange.duration().rescaled_to(timeRange.duration().rate());
                        if (math::fuzzyCompare(
                                audioStartTime.value(),
                                timeRange.start_time().value()) &&
                            math::fuzzyCompare(
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
                        const otime::TimeRange& videoTimeRange = 
                            tracks[trackIndex - 1].items[i]->getTimeRange();
                        const otime::RationalTime videoStartTime =
                            videoTimeRange.start_time().rescaled_to(timeRange.start_time().rate());
                        const otime::RationalTime videoDuration =
                            videoTimeRange.duration().rescaled_to(timeRange.duration().rate());
                        if (math::fuzzyCompare(
                                videoStartTime.value(),
                                timeRange.start_time().value()) &&
                            math::fuzzyCompare(
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
            const math::Box2i& geometry,
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
                    math::Box2i g;
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
                        dt.mouse = math::Box2i(
                            g.min.x - size.handle,
                            g.min.y,
                            size.handle * 2,
                            g.h());
                        dt.draw = math::Box2i(
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
                        dt.mouse = math::Box2i(
                            g.max.x - size.handle,
                            g.min.y,
                            size.handle * 2,
                            g.h());
                        dt.draw = math::Box2i(
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
