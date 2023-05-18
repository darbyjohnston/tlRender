// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItem.h>

#include <tlTimelineUI/TrackItem.h>

#include <tlUI/ScrollArea.h>

#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

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
                p.size.margin +
                p.size.border +
                p.size.margin;
            for (const auto& child : _children)
            {
                const auto& sizeHint = child->getSizeHint();
                child->setGeometry(math::BBox2i(
                    _geometry.min.x + p.size.margin,
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
                p.size.margin +
                p.timeRange.duration().rescaled_to(1.0).value() * _scale +
                p.size.margin,
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.margin +
                p.size.border * 4 +
                p.size.margin +
                p.size.border +
                p.size.margin +
                childrenHeight +
                p.size.margin);
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
                p.size.border * 4 +
                p.size.margin;
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
                p.mouse.pressed = true;
                p.mouse.pressPos = event.pos;
                if (p.stopOnScrub)
                {
                    p.player->setPlayback(timeline::Playback::Stop);
                }
                const math::BBox2i bbox = _geometry.margin(-p.size.margin);
                if (bbox.contains(event.pos))
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

        void TimelineItem::keyPressEvent(ui::KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled())
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
                }
            }
        }

        void TimelineItem::keyReleaseEvent(ui::KeyEvent& event)
        {
            event.accept = true;
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

                const int x0 = _timeToPos(_p->inOutRange.start_time());
                const int x1 = _timeToPos(_p->inOutRange.end_time_inclusive());

                const math::BBox2i bbox(
                    x0,
                    p.size.scrollPos.y +
                    g.min.y +
                    p.size.margin,
                    x1 - x0 + 1,
                    p.size.fontMetrics.lineHeight +
                    p.size.margin);
                event.render->drawRect(
                    bbox,
                    imaging::Color4f(1.F, .7F, .2F, .1F));
            }
        }

        void TimelineItem::_drawTimeTicks(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const math::BBox2i& g = _geometry;
            const int handle = event.style->getSizeRole(ui::SizeRole::Handle, event.displayScale);

            const int w = _sizeHint.x - p.size.margin * 2;
            const int frameTick = 1.0 /
                p.timeRange.duration().value() * w;
            const int secondsTick = 1.0 /
                p.timeRange.duration().rescaled_to(1.0).value() * w;
            const int minutesTick = 60.0 /
                p.timeRange.duration().rescaled_to(1.0).value() * w;
            const int hoursTick = 3600.0 /
                p.timeRange.duration().rescaled_to(1.0).value() * w;
            double seconds = 0;
            int tick = 0;
            if (frameTick >= handle)
            {
                seconds = 1.0 / p.timeRange.duration().rate();
                tick = frameTick;
            }
            else if (secondsTick >= handle)
            {
                seconds = 1.0;
                tick = secondsTick;
            }
            else if (minutesTick >= handle)
            {
                seconds = 60.0;
                tick = minutesTick;
            }
            else if (hoursTick >= handle)
            {
                seconds = 3600.0;
                tick = hoursTick;
            }
            if (seconds > 0.0 && tick > 0)
            {
                const float duration = p.timeRange.duration().rescaled_to(1.0).value();
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (double t = 0.0; t < duration; t += seconds)
                {
                    const math::BBox2i bbox(
                        g.min.x +
                        p.size.margin +
                        t / duration * w,
                        p.size.scrollPos.y +
                        g.min.y +
                        p.size.margin,
                        2,
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
            }

            /*if (frameWidth >= handle)
            {
                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (double t = 0.0; t < p.timeRange.duration().value(); t += 1.0)
                {
                    math::BBox2i bbox(
                        g.min.x +
                        p.size.margin +
                        t / p.timeRange.duration().value() * (_sizeHint.x - p.size.margin * 2),
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.spacing +
                        p.size.fontMetrics.lineHeight / 2,
                        1,
                        p.size.fontMetrics.lineHeight / 2);
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
                        imaging::Color4f(.6F, .6F, .6F));
                }
            }

            if (secondsWidth >= handle)
            {
                std::string labelMax = _timeLabel(p.timeRange.end_time_inclusive(), _options.timeUnits);
                math::Vector2i labelMaxSize = event.fontSystem->getSize(labelMax, fontInfo);
                if (labelMaxSize.x < (secondsWidth - p.size.spacing))
                {
                    for (double t = 0.0;
                        t < p.timeRange.duration().value();
                        t += p.timeRange.duration().rate())
                    {
                        math::BBox2i bbox(
                            g.min.x +
                            p.size.margin +
                            t / p.timeRange.duration().value() * (_sizeHint.x - p.size.margin * 2),
                            g.min.y +
                            p.size.margin +
                            p.size.fontMetrics.lineHeight +
                            p.size.spacing +
                            p.size.fontMetrics.lineHeight +
                            p.size.spacing,
                            labelMaxSize.x,
                            p.size.fontMetrics.lineHeight);
                        if (bbox.intersects(drawRect))
                        {
                            std::string label = _timeLabel(
                                p.timeRange.start_time() + otime::RationalTime(t, p.timeRange.duration().rate()),
                                _options.timeUnits);
                            event.render->drawText(
                                event.fontSystem->getGlyphs(label, fontInfo),
                                math::Vector2i(
                                    bbox.min.x,
                                    bbox.min.y +
                                    p.size.fontMetrics.ascender),
                                event.style->getColorRole(ColorRole::Text));
                        }
                    }
                }

                geom::TriangleMesh2 mesh;
                size_t i = 1;
                for (double t = 0.0;
                    t < p.timeRange.duration().value();
                    t += p.timeRange.duration().rate())
                {
                    math::BBox2i bbox(
                        g.min.x +
                        p.size.margin + t / p.timeRange.duration().value() * (_sizeHint.x - p.size.margin * 2),
                        g.min.y +
                        p.size.margin +
                        p.size.fontMetrics.lineHeight +
                        p.size.spacing,
                        2,
                        p.size.fontMetrics.lineHeight);
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
                        imaging::Color4f(.8F, .8F, .8F));
                }
            }*/
        }

        void TimelineItem::_drawCacheInfo(
            const math::BBox2i& drawRect,
            const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

            geom::TriangleMesh2 mesh;
            size_t i = 1;
            for (const auto& t : p.cacheInfo.videoFrames)
            {
                const int x0 = _timeToPos(t.start_time());
                const int x1 = _timeToPos(t.end_time_inclusive());
                const math::BBox2i bbox(
                    x0,
                    p.size.scrollPos.y +
                    g.min.y +
                    p.size.margin +
                    p.size.fontMetrics.lineHeight +
                    p.size.margin,
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
                    imaging::Color4f(.2F, .4F, .4F));
            }

            mesh.v.clear();
            mesh.triangles.clear();
            i = 1;
            for (const auto& t : p.cacheInfo.audioFrames)
            {
                const int x0 = _timeToPos(t.start_time());
                const int x1 = _timeToPos(t.end_time_inclusive());
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
                    imaging::Color4f(.3F, .25F, .4F));
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
                math::Vector2i pos(
                    _timeToPos(currentTime),
                    p.size.scrollPos.y +
                    g.min.y);

                /*geom::TriangleMesh2 mesh;
                mesh.v.push_back(math::Vector2f(
                    pos.x -
                    p.size.fontMetrics.lineHeight / 3,
                    pos.y +
                    p.size.fontMetrics.lineHeight +
                    p.size.spacing));
                mesh.v.push_back(math::Vector2f(
                    pos.x +
                    p.size.fontMetrics.lineHeight / 3,
                    pos.y +
                    p.size.fontMetrics.lineHeight +
                    p.size.spacing));
                mesh.v.push_back(math::Vector2f(
                    pos.x,
                    pos.y +
                    p.size.fontMetrics.lineHeight +
                    p.size.spacing +
                    p.size.fontMetrics.lineHeight / 2));
                mesh.triangles.push_back(geom::Triangle2({ 1, 2, 3 }));
                event.render->drawMesh(
                    mesh,
                    math::Vector2i(),
                    event.style->getColorRole(ui::ColorRole::Text));*/
                event.render->drawRect(
                    math::BBox2i(
                        pos.x - p.size.border,
                        pos.y,
                        p.size.border * 2,
                        g.h()),
                    event.style->getColorRole(ui::ColorRole::Red));

                std::string label = _timeLabel(currentTime, _options.timeUnits);
                event.render->drawText(
                    event.fontSystem->getGlyphs(label, fontInfo),
                    math::Vector2i(
                        pos.x + p.size.spacing,
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
            const math::BBox2i bbox = _geometry.margin(-p.size.margin);
            if (bbox.w() > 0)
            {
                const double normalized =
                    (value - bbox.min.x) / static_cast<double>(bbox.w());
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
            int out = 0;
            if (!time::compareExact(value, time::invalidTime) &&
                p.timeRange.duration().value() > 0.0)
            {
                const math::BBox2i bbox = _geometry.margin(-p.size.margin);
                const float normalized =
                    (value.value() - p.timeRange.start_time().value()) /
                    p.timeRange.duration().value();
                out = bbox.min.x + normalized * bbox.w();
            }
            return out;
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