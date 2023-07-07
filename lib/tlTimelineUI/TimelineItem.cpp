// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItem.h>

#include <tlTimelineUI/TrackItem.h>

#include <tlUI/ScrollArea.h>

#include <tlTimeline/Util.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineItem::Private
        {
            std::shared_ptr<timeline::Player> player;
            otime::TimeRange timeRange = time::invalidTimeRange;
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            timeline::PlayerCacheInfo cacheInfo;
            bool stopOnScrub = true;
            ui::FontRole fontRole = ui::FontRole::Mono;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                int border = 0;
                imaging::FontMetrics fontMetrics;
                math::Vector2i scrollPos;
            };
            SizeData size;

            struct MouseData
            {
                bool pressed = false;
                math::Vector2i pressPos;
                bool currentTimeDrag = false;
            };
            MouseData mouse;

            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> > inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;
        };

        void TimelineItem::_init(
            const std::shared_ptr<timeline::Player>& player,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init("tl::timelineui::TimelineItem", itemData, context, parent);
            TLRENDER_P();

            p.player = player;
            p.timeRange = player->getTimeRange();

            const auto otioTimeline = p.player->getTimeline()->getTimeline();
            for (const auto& child : otioTimeline->tracks()->children())
            {
                if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                {
                    auto trackItem = TrackItem::create(
                        track,
                        itemData,
                        context,
                        shared_from_this());
                }
            }

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
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
            out->_init(player, itemData, context, parent);
            return out;
        }

        void TimelineItem::setStopOnScrub(bool value)
        {
            _p->stopOnScrub = value;
        }

        void TimelineItem::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void TimelineItem::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void TimelineItem::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();

            float y =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border;
            for (const auto& child : _children)
            {
                const auto& sizeHint = child->getSizeHint();
                child->setGeometry(math::BBox2i(
                    _geometry.min.x,
                    _geometry.min.y + y,
                    sizeHint.x,
                    sizeHint.y));
                y += sizeHint.y;;
            }

            if (auto scrollArea = getParentT<ui::ScrollArea>())
            {
                p.size.scrollPos = scrollArea->getScrollPos();
            }
        }

        void TimelineItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);
            p.size.spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall, event.displayScale);
            p.size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);

            p.size.fontMetrics = event.getFontMetrics(p.fontRole);

            int childrenHeight = 0;
            for (const auto& child : _children)
            {
                childrenHeight += child->getSizeHint().y;
            }

            _sizeHint = math::Vector2i(
                p.timeRange.duration().rescaled_to(1.0).value() * _scale,
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.border +
                childrenHeight);
        }

        void TimelineItem::clipEvent(
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

        void TimelineItem::drawOverlayEvent(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawOverlayEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

            int y =
                p.size.scrollPos.y +
                g.min.y;
            int h =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4;
            event.render->drawRect(
                math::BBox2i(g.min.x, y, g.w(), h),
                event.style->getColorRole(ui::ColorRole::Base));

            y = y + h;
            h = p.size.border;
            event.render->drawRect(
                math::BBox2i(g.min.x, y, g.w(), h),
                event.style->getColorRole(ui::ColorRole::Border));

            _drawInOutPoints(drawRect, event);
            _drawTimeTicks(drawRect, event);
            _drawCacheInfo(drawRect, event);
            _drawCurrentTime(drawRect, event);
        }

        void TimelineItem::enterEvent()
        {}

        void TimelineItem::leaveEvent()
        {}

        void TimelineItem::mouseMoveEvent(ui::MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            if (p.mouse.currentTimeDrag)
            {
                p.player->seek(_posToTime(event.pos.x));
            }
        }

        void TimelineItem::mousePressEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                event.accept = true;
                takeKeyFocus();
                p.mouse.pressed = true;
                p.mouse.pressPos = event.pos;
                if (p.stopOnScrub)
                {
                    p.player->setPlayback(timeline::Playback::Stop);
                }
                if (_geometry.contains(event.pos))
                {
                    p.mouse.currentTimeDrag = true;
                    p.player->seek(_posToTime(event.pos.x));
                }
            }
        }

        void TimelineItem::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
            p.mouse.currentTimeDrag = false;
        }

        /*void TimelineItem::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && 0 == event.modifiers)
            {
                switch (event.key)
                {
                case ui::Key::Space:
                    event.accept = true;
                    switch (p.player->observePlayback()->get())
                    {
                    case timeline::Playback::Stop:
                        p.player->setPlayback(timeline::Playback::Forward);
                        break;
                    case timeline::Playback::Forward:
                    case timeline::Playback::Reverse:
                        p.player->setPlayback(timeline::Playback::Stop);
                        break;
                    default: break;
                    }
                    break;
                case ui::Key::Up:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::FrameNextX10);
                    break;
                case ui::Key::Down:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::FramePrevX10);
                    break;
                case ui::Key::Right:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::FrameNext);
                    break;
                case ui::Key::Left:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::FramePrev);
                    break;
                case ui::Key::PageUp:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::FrameNextX100);
                    break;
                case ui::Key::PageDown:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::FramePrevX100);
                    break;
                case ui::Key::Home:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::Start);
                    break;
                case ui::Key::End:
                    event.accept = true;
                    p.player->timeAction(timeline::TimeAction::End);
                    break;
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
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void TimelineItem::_drawInOutPoints(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();
            if (!time::compareExact(_p->inOutRange, time::invalidTimeRange) &&
                !time::compareExact(_p->inOutRange, _p->timeRange))
            {
                const math::BBox2i& g = _geometry;

                switch (_options.inOutDisplay)
                {
                case InOutDisplay::InsideRange:
                {
                    const int x0 = _timeToPos(_p->inOutRange.start_time());
                    const int x1 = _timeToPos(_p->inOutRange.end_time_exclusive());
                    const math::BBox2i bbox(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(
                        bbox,
                        _options.colors[ColorRole::InOut]);
                    break;
                }
                case InOutDisplay::OutsideRange:
                {
                    int x0 = _timeToPos(_p->timeRange.start_time());
                    int x1 = _timeToPos(_p->inOutRange.start_time());
                    math::BBox2i bbox(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(
                        bbox,
                        _options.colors[ColorRole::InOut]);
                    x0 = _timeToPos(_p->inOutRange.end_time_exclusive());
                    x1 = _timeToPos(_p->timeRange.end_time_exclusive());
                    bbox = math::BBox2i(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y,
                        x1 - x0 + 1,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin);
                    event.render->drawRect(
                        bbox,
                        _options.colors[ColorRole::InOut]);
                    break;
                }
                default: break;
                }
            }
        }

        void TimelineItem::_drawTimeTicks(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const int handle = event.style->getSizeRole(ui::SizeRole::Handle, event.displayScale);
            const math::BBox2i& g = _geometry;

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const std::string labelMax = _data.timeUnitsModel->getLabel(p.timeRange.duration());
            const math::Vector2i labelMaxSize = event.fontSystem->getSize(labelMax, fontInfo);
            const int distanceMin = p.size.border + p.size.spacing + labelMaxSize.x;

            const int w = _sizeHint.x;
            const float duration = p.timeRange.duration().rescaled_to(1.0).value();
            const int frameTick = 1.0 / p.timeRange.duration().value() * w;
            if (frameTick >= handle)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (double t = 0.0; t < duration; t += 1.0 / p.timeRange.duration().rate())
                {
                    const math::BBox2i bbox(
                        g.min.x +
                        t / duration * w,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight,
                        p.size.border,
                        p.size.margin +
                        p.size.border * 4);
                    if (bbox.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.max.y + 1));
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.max.y + 1));
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
                    const math::BBox2i bbox(
                        g.min.x +
                        t / duration * w,
                        p.size.scrollPos.y +
                        g.min.y,
                        p.size.border,
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 4);
                    if (bbox.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.max.y + 1));
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.max.y + 1));
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

                for (double t = 0.0; t < duration; t += seconds)
                {
                    const math::BBox2i bbox(
                        g.min.x +
                        t / duration * w +
                        p.size.border +
                        p.size.spacing,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin,
                        labelMaxSize.x,
                        p.size.fontMetrics.lineHeight);
                    if (bbox.intersects(drawRect))
                    {
                        const std::string label = _data.timeUnitsModel->getLabel(
                            p.timeRange.start_time() +
                            otime::RationalTime(t, 1.0).rescaled_to(p.timeRange.duration().rate()));
                        event.render->drawText(
                            event.fontSystem->getGlyphs(label, fontInfo),
                            math::Vector2i(
                                bbox.min.x,
                                bbox.min.y +
                                p.size.fontMetrics.ascender),
                            event.style->getColorRole(ui::ColorRole::Button));
                    }
                }
            }
        }

        void TimelineItem::_drawCacheInfo(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

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
                    const math::BBox2i bbox(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin,
                        x1 - x0 + 1,
                        h);
                    if (bbox.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.max.y + 1));
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.max.y + 1));
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
                        _options.colors[ColorRole::VideoCache]);
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
                    const math::BBox2i bbox(
                        x0,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.margin +
                        p.size.border * 2,
                        x1 - x0 + 1,
                        p.size.border * 2);
                    if (bbox.intersects(drawRect))
                    {
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.min.y));
                        mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.max.y + 1));
                        mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.max.y + 1));
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
                        _options.colors[ColorRole::AudioCache]);
                }
            }
        }

        void TimelineItem::_drawCurrentTime(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const math::BBox2i& g = _geometry;

            const otime::RationalTime& currentTime = p.player->observeCurrentTime()->get();
            if (!time::compareExact(currentTime, time::invalidTime))
            {
                const math::Vector2i pos(
                    _timeToPos(currentTime),
                    p.size.scrollPos.y +
                    g.min.y);

                event.render->drawRect(
                    math::BBox2i(
                        pos.x,
                        pos.y,
                        p.size.border * 2,
                        g.h()),
                    event.style->getColorRole(ui::ColorRole::Red));

                const std::string label = _data.timeUnitsModel->getLabel(currentTime);
                event.render->drawText(
                    event.fontSystem->getGlyphs(label, fontInfo),
                    math::Vector2i(
                        pos.x + p.size.border * 2 + p.size.spacing,
                        pos.y +
                        p.size.margin +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        otime::RationalTime TimelineItem::_posToTime(float value) const
        {
            TLRENDER_P();
            otime::RationalTime out = time::invalidTime;
            if (_geometry.w() > 0)
            {
                const double normalized =
                    (value - _geometry.min.x) / static_cast<double>(_geometry.w());
                out = time::round(
                    p.timeRange.start_time() +
                    otime::RationalTime(
                        p.timeRange.duration().value() * normalized,
                        p.timeRange.duration().rate()));
                out = math::clamp(
                    out,
                    p.timeRange.start_time(),
                    p.timeRange.end_time_inclusive());
            }
            return out;
        }

        int TimelineItem::_timeToPos(const otime::RationalTime& value) const
        {
            TLRENDER_P();
            const otime::RationalTime t = value - p.timeRange.start_time();
            return _geometry.min.x + t.rescaled_to(1.0).value() * _scale;
        }

        void TimelineItem::_resetMouse()
        {
            TLRENDER_P();
            if (p.mouse.pressed || p.mouse.currentTimeDrag)
            {
                p.mouse.pressed = false;
                p.mouse.currentTimeDrag = false;
                _updates |= ui::Update::Draw;
            }
        }
    }
}
