// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/TimelineItem.h>

#include <tlUI/TimelineTrackItem.h>

#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        struct TimelineItem::Private
        {
            std::shared_ptr<timeline::TimelinePlayer> timelinePlayer;
            otime::TimeRange timeRange = time::invalidTimeRange;
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            timeline::PlayerCacheInfo cacheInfo;
            bool stopOnScrub = true;
            ui::FontRole fontRole = ui::FontRole::Label;

            struct SizeData
            {
                int margin = 0;
                int spacing = 0;
                imaging::FontMetrics fontMetrics;
            };
            SizeData size;

            struct MouseData
            {
                bool press = false;
                math::Vector2i pressPos;
                bool currentTimeDrag = false;
            };
            MouseData mouse;

            std::shared_ptr<observer::ValueObserver<otime::RationalTime> > currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> > inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;
        };

        void TimelineItem::_init(
            const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ITimelineItem::_init("TimelineItem", itemData, context, parent);
            TLRENDER_P();

            p.timelinePlayer = timelinePlayer;
            p.timeRange = timelinePlayer->getTimeRange();

            const auto otioTimeline = p.timelinePlayer->getTimeline()->getTimeline();
            for (const auto& child : otioTimeline->tracks()->children())
            {
                if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                {
                    auto trackItem = TimelineTrackItem::create(
                        track,
                        itemData,
                        context,
                        shared_from_this());
                }
            }

            p.currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                p.timelinePlayer->observeCurrentTime(),
                [this](const otime::RationalTime& value)
                {
                    _p->currentTime = value;
                    _updates |= ui::Update::Draw;
                });

            p.inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                p.timelinePlayer->observeInOutRange(),
                [this](const otime::TimeRange value)
                {
                    _p->inOutRange = value;
                    _updates |= ui::Update::Draw;
                });

            p.cacheInfoObserver = observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                p.timelinePlayer->observeCacheInfo(),
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
            const std::shared_ptr<timeline::TimelinePlayer>& timelinePlayer,
            const TimelineItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
            out->_init(timelinePlayer, itemData, context, parent);
            return out;
        }

        void TimelineItem::setStopOnScrub(bool value)
        {
            _p->stopOnScrub = value;
        }

        void TimelineItem::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();

            float y =
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.spacing +
                p.size.fontMetrics.lineHeight +
                p.size.spacing +
                p.size.fontMetrics.lineHeight +
                p.size.spacing;
            for (const auto& child : _children)
            {
                const auto& sizeHint = child->getSizeHint();
                child->setGeometry(math::BBox2i(
                    _geometry.min.x + p.size.margin,
                    _geometry.min.y + y,
                    sizeHint.x,
                    sizeHint.y));
                y += sizeHint.y;// +_spacing;
            }
        }

        void TimelineItem::sizeEvent(const ui::SizeEvent& event)
        {
            ITimelineItem::sizeEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(ui::SizeRole::MarginSmall, event.displayScale);
            p.size.spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall, event.displayScale);
            p.size.fontMetrics = event.getFontMetrics(p.fontRole);

            int childrenHeight = 0;
            for (const auto& child : _children)
            {
                childrenHeight += child->getSizeHint().y;
            }
            //if (!_children.empty())
            //{
            //    childrenHeight += (_children.size() - 1) * _spacing;
            //}

            _sizeHint = math::Vector2i(
                p.size.margin +
                p.timeRange.duration().rescaled_to(1.0).value() * _options.scale +
                p.size.margin,
                p.size.margin +
                p.size.fontMetrics.lineHeight +
                p.size.spacing +
                p.size.fontMetrics.lineHeight +
                p.size.spacing +
                p.size.fontMetrics.lineHeight +
                p.size.spacing +
                childrenHeight +
                p.size.margin);
        }

        void TimelineItem::drawEvent(const ui::DrawEvent& event)
        {
            ITimelineItem::drawEvent(event);
            _drawTimeTicks(event);
            _drawCurrentTime(event);
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
                p.timelinePlayer->seek(_posToTime(event.pos.x));
            }
        }

        void TimelineItem::mousePressEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            if (0 == event.modifiers)
            {
                event.accept = true;
                p.mouse.press = true;
                p.mouse.pressPos = event.pos;
                if (p.stopOnScrub)
                {
                    p.timelinePlayer->setPlayback(timeline::Playback::Stop);
                }
                const math::BBox2i bbox = _getCurrentTimeBBox();
                if (bbox.contains(event.pos))
                {
                    p.mouse.currentTimeDrag = true;
                    p.timelinePlayer->seek(_posToTime(event.pos.x));
                }
            }
        }

        void TimelineItem::mouseReleaseEvent(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.press = false;
            p.mouse.currentTimeDrag = false;
        }

        void TimelineItem::_drawTimeTicks(const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const math::BBox2i g = _geometry;

            const float frameTick0 = p.timeRange.start_time().value() /
                p.timeRange.duration().value() * (_sizeHint.x - p.size.margin * 2);
            const float frameTick1 = (p.timeRange.start_time().value() + 1.0) /
                p.timeRange.duration().value() * (_sizeHint.x - p.size.margin * 2);
            const int frameWidth = frameTick1 - frameTick0;
            if (frameWidth >= 5)
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
                    if (bbox.intersects(_viewport))
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

            const float secondsTick0 = p.timeRange.start_time().value() /
                (p.timeRange.duration().value() / p.timeRange.duration().rate()) * (_sizeHint.x - p.size.margin * 2);
            const float secondsTick1 = (p.timeRange.start_time().value() + 1.0) /
                (p.timeRange.duration().value() / p.timeRange.duration().rate()) * (_sizeHint.x - p.size.margin * 2);
            const int secondsWidth = secondsTick1 - secondsTick0;
            if (secondsWidth >= 5)
            {
                std::string labelMax = _timeLabel(p.timeRange.end_time_inclusive(), _options.timeUnits);
                math::Vector2i labelMaxSize = event.fontSystem->measure(labelMax, fontInfo);
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
                        if (bbox.intersects(_viewport))
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
                                event.style->getColorRole(ui::ColorRole::Text));
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
                    if (bbox.intersects(_viewport))
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
            }
        }

        void TimelineItem::_drawCurrentTime(const ui::DrawEvent& event)
        {
            TLRENDER_P();

            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            const math::BBox2i g = _geometry;

            const otime::RationalTime& currentTime = p.timelinePlayer->observeCurrentTime()->get();
            if (!time::compareExact(currentTime, time::invalidTime))
            {
                math::Vector2i pos(
                    _timeToPos(currentTime),
                    g.min.y +
                    p.size.margin);

                geom::TriangleMesh2 mesh;
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
                    event.style->getColorRole(ui::ColorRole::Text));

                std::string label = _timeLabel(currentTime, _options.timeUnits);
                event.render->drawText(
                    event.fontSystem->getGlyphs(label, fontInfo),
                    math::Vector2i(
                        pos.x,
                        pos.y +
                        p.size.fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }
        }

        math::BBox2i TimelineItem::_getCurrentTimeBBox() const
        {
            TLRENDER_P();
            return math::BBox2i(
                _geometry.min.x + p.size.margin,
                _geometry.min.y + p.size.margin,
                _geometry.w() - p.size.margin * 2,
                _geometry.h() - p.size.margin * 2);
        }

        otime::RationalTime TimelineItem::_posToTime(float value) const
        {
            TLRENDER_P();
            otime::RationalTime out = time::invalidTime;
            const math::BBox2i bbox = _getCurrentTimeBBox();
            if (bbox.w() > 0)
            {
                const float v = (value - bbox.min.x) / static_cast<float>(bbox.w());
                out = time::round(
                    p.timeRange.start_time() +
                    otime::RationalTime(
                        p.timeRange.duration().value() * v,
                        p.timeRange.duration().rate()));
            }
            return out;
        }

        float TimelineItem::_timeToPos(const otime::RationalTime& value) const
        {
            TLRENDER_P();
            float out = 0.F;
            const otime::RationalTime& currentTime = p.timelinePlayer->observeCurrentTime()->get();
            if (!time::compareExact(currentTime, time::invalidTime))
            {
                const math::BBox2i bbox = _getCurrentTimeBBox();
                out = bbox.min.x +
                    (currentTime.value() - p.timeRange.start_time().value()) /
                    p.timeRange.duration().value() *
                    bbox.w();
            }
            return out;
        }
    }
}