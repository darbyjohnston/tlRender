// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineItem.h"

#include "TrackItem.h"

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void TimelineItem::_init(
                const std::shared_ptr<timeline::Timeline>& timeline,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                BaseItem::_init(itemData, context);

                _timeline = timeline;
                _timeRange = timeline->getTimeRange();

                const auto otioTimeline = timeline->getTimeline();
                for (const auto& child : otioTimeline->tracks()->children())
                {
                    if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                    {
                        auto trackItem = TrackItem::create(track, itemData, context);
                        _children.push_back(trackItem);
                    }
                }

                _label = _nameLabel(otioTimeline->name());
                _durationLabel = BaseItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());
            }

            std::shared_ptr<TimelineItem> TimelineItem::create(
                const std::shared_ptr<timeline::Timeline>& timeline,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
                out->_init(timeline, itemData, context);
                return out;
            }

            TimelineItem::~TimelineItem()
            {}

            void TimelineItem::preLayout()
            {
                int childrenHeight = 0;
                for (const auto& child : _children)
                {
                    childrenHeight += child->sizeHint().y;
                }

                _sizeHint = math::Vector2i(
                    _itemData.margin +
                    _timeRange.duration().rescaled_to(1.0).value() * _scale +
                    _itemData.margin,
                    _itemData.margin +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _thumbnailHeight +
                    childrenHeight +
                    _itemData.margin);
            }

            void TimelineItem::layout(const math::BBox2i& geometry)
            {
                BaseItem::layout(geometry);

                const auto& info = _timeline->getIOInfo();
                _thumbnailWidth = !info.video.empty() ?
                    static_cast<int>(_thumbnailHeight * info.video[0].size.getAspect()) :
                    0;

                float y =
                    _itemData.margin +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _itemData.fontMetrics.lineHeight +
                    _itemData.spacing +
                    _thumbnailHeight;
                for (const auto& child : _children)
                {
                    const auto& sizeHint = child->sizeHint();
                    child->layout(math::BBox2i(
                        _geometry.min.x + _itemData.margin,
                        _geometry.min.y + y,
                        sizeHint.x,
                        sizeHint.y));
                    y += sizeHint.y;
                }

                _timeline->cancelRequests();
                _videoDataFutures.clear();
            }

            void TimelineItem::render(
                const std::shared_ptr<timeline::IRender>& render,
                const math::BBox2i& viewport,
                float devicePixelRatio)
            {
                BaseItem::render(render, viewport, devicePixelRatio);

                if (viewport != _viewportTmp)
                {
                    _viewportTmp = viewport;
                    _timeline->cancelRequests();
                    _videoDataFutures.clear();
                }

                const math::BBox2i g(
                    _geometry.min.x - viewport.min.x,
                    _geometry.min.y - viewport.min.y,
                    _geometry.w(),
                    _geometry.h());
                const math::BBox2i v(
                    0, 0, viewport.w(), viewport.h());
                if (g.intersects(v))
                {
                    render->drawRect(
                        g * devicePixelRatio,
                        imaging::Color4f(.15, .15, .15));

                    auto fontInfo = _itemData.fontInfo;
                    fontInfo.size *= devicePixelRatio;
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_label, fontInfo),
                        math::Vector2i(
                            g.min.x +
                            _itemData.margin,
                            g.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_startLabel, fontInfo),
                        math::Vector2i(
                            g.min.x +
                            _itemData.margin,
                            g.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));

                    math::Vector2i textSize = _itemData.fontSystem->measure(_durationLabel, _itemData.fontInfo);
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_durationLabel, fontInfo),
                        math::Vector2i(
                            g.max.x -
                            _itemData.margin -
                            textSize.x,
                            g.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));
                    textSize = _itemData.fontSystem->measure(_endLabel, _itemData.fontInfo);
                    render->drawText(
                        _itemData.fontSystem->getGlyphs(_endLabel, fontInfo),
                        math::Vector2i(
                            g.max.x -
                            _itemData.margin -
                            textSize.x,
                            g.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                        imaging::Color4f(.9F, .9F, .9F));

                    const float frameTick0 = _timeRange.start_time().value() /
                        _timeRange.duration().value() * (g.w() - _itemData.margin * 2);
                    const float frameTick1 = (_timeRange.start_time().value() + 1.0) /
                        _timeRange.duration().value() * (g.w() - _itemData.margin * 2);
                    const int frameWidth = frameTick1 - frameTick0;
                    if (frameWidth >= _itemData.minTickSpacing)
                    {
                        std::string labelMax = string::Format("{0}").arg(_timeRange.end_time_inclusive().value());
                        math::Vector2i labelMaxSize = _itemData.fontSystem->measure(labelMax, _itemData.fontInfo);
                        if (labelMaxSize.x < (frameWidth - _itemData.spacing))
                        {
                            for (double t = 1.0; t < _timeRange.duration().value(); t += 1.0)
                            {
                                const math::BBox2i bbox(
                                    g.min.x +
                                    _itemData.margin + t / _timeRange.duration().value() * (g.w() - _itemData.margin * 2),
                                    g.min.y +
                                    _itemData.margin +
                                    _itemData.fontMetrics.lineHeight +
                                    _itemData.spacing +
                                    _itemData.fontMetrics.lineHeight +
                                    _itemData.spacing,
                                    labelMaxSize.x,
                                    _itemData.fontMetrics.lineHeight);
                                if (bbox.intersects(v))
                                {
                                    std::string label = string::Format("{0}").arg(t);
                                    render->drawText(
                                        _itemData.fontSystem->getGlyphs(label, fontInfo),
                                        math::Vector2i(
                                            bbox.min.x,
                                            bbox.min.y +
                                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                                        imaging::Color4f(.9F, .9F, .9F));
                                }
                            }
                        }

                        geom::TriangleMesh2 mesh;
                        size_t i = 1;
                        for (double t = 1.0; t < _timeRange.duration().value(); t += 1.0)
                        {
                            const math::BBox2i bbox(
                                g.min.x +
                                _itemData.margin + t / _timeRange.duration().value() * (g.w() - _itemData.margin * 2),
                                g.min.y +
                                _itemData.margin +
                                _itemData.fontMetrics.lineHeight +
                                _itemData.spacing +
                                _itemData.fontMetrics.lineHeight +
                                _itemData.spacing +
                                _itemData.fontMetrics.lineHeight +
                                _itemData.spacing,
                                1,
                                _itemData.fontMetrics.lineHeight);
                            if (bbox.intersects(v))
                            {
                                mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.min.y) * devicePixelRatio);
                                mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.min.y) * devicePixelRatio);
                                mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.max.y + 1) * devicePixelRatio);
                                mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.max.y + 1) * devicePixelRatio);
                                mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                                mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                                i += 4;
                            }
                        }
                        if (!mesh.v.empty())
                        {
                            render->drawMesh(
                                mesh,
                                imaging::Color4f(.6F, .6F, .6F));
                        }
                    }

                    const float secondsTick0 = _timeRange.start_time().value() /
                        (_timeRange.duration().value() / _timeRange.duration().rate()) * (g.w() - _itemData.margin * 2);
                    const float secondsTick1 = (_timeRange.start_time().value() + 1.0) /
                        (_timeRange.duration().value() / _timeRange.duration().rate()) * (g.w() - _itemData.margin * 2);
                    const int secondsWidth = secondsTick1 - secondsTick0;
                    if (secondsWidth >= _itemData.minTickSpacing)
                    {
                        std::string labelMax = string::Format("{0}").arg(_timeRange.end_time_inclusive().value());
                        math::Vector2i labelMaxSize = _itemData.fontSystem->measure(labelMax, _itemData.fontInfo);
                        if (labelMaxSize.x < (secondsWidth - _itemData.spacing))
                        {
                            for (double t = 0.0;
                                t < _timeRange.duration().value();
                                t += _timeRange.duration().rate())
                            {
                                const math::BBox2i bbox(
                                    g.min.x +
                                    _itemData.margin +
                                    t / _timeRange.duration().value() * (g.w() - _itemData.margin * 2),
                                    g.min.y +
                                    _itemData.margin +
                                    _itemData.fontMetrics.lineHeight +
                                    _itemData.spacing +
                                    _itemData.fontMetrics.lineHeight +
                                    _itemData.spacing,
                                    labelMaxSize.x,
                                    _itemData.fontMetrics.lineHeight);
                                if (bbox.intersects(v))
                                {
                                    std::string label = string::Format("{0}").arg(t);
                                    render->drawText(
                                        _itemData.fontSystem->getGlyphs(label, fontInfo),
                                        math::Vector2i(
                                            bbox.min.x,
                                            bbox.min.y +
                                            _itemData.fontMetrics.ascender) * devicePixelRatio,
                                        imaging::Color4f(.9F, .9F, .9F));
                                }
                            }
                        }

                        geom::TriangleMesh2 mesh;
                        size_t i = 1;
                        for (double t = 0.0;
                            t < _timeRange.duration().value();
                            t += _timeRange.duration().rate())
                        {
                            const math::BBox2i bbox(
                                g.min.x +
                                _itemData.margin + t / _timeRange.duration().value() * (g.w() - _itemData.margin * 2),
                                g.min.y +
                                _itemData.margin +
                                _itemData.fontMetrics.lineHeight +
                                _itemData.spacing +
                                _itemData.fontMetrics.lineHeight +
                                _itemData.spacing +
                                _itemData.fontMetrics.lineHeight +
                                _itemData.spacing,
                                1,
                                _itemData.fontMetrics.lineHeight);
                            if (bbox.intersects(v))
                            {
                                mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.min.y) * devicePixelRatio);
                                mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.min.y) * devicePixelRatio);
                                mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.max.y + 1) * devicePixelRatio);
                                mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.max.y + 1) * devicePixelRatio);
                                mesh.triangles.push_back({ i + 0, i + 1, i + 2 });
                                mesh.triangles.push_back({ i + 2, i + 3, i + 0 });
                                i += 4;
                            }
                        }
                        if (!mesh.v.empty())
                        {
                            render->drawMesh(
                                mesh,
                                imaging::Color4f(.8F, .8F, .8F));
                        }
                    }

                    const math::BBox2i bbox(
                        g.min.x +
                        _itemData.margin,
                        g.min.y +
                        _itemData.margin +
                        _itemData.fontMetrics.lineHeight +
                        _itemData.spacing +
                        _itemData.fontMetrics.lineHeight +
                        _itemData.spacing +
                        _itemData.fontMetrics.lineHeight +
                        _itemData.spacing +
                        _itemData.fontMetrics.lineHeight +
                        _itemData.spacing,
                        g.w() - _itemData.margin * 2,
                        _thumbnailHeight);
                    render->drawRect(
                        bbox * devicePixelRatio,
                        imaging::Color4f(0.F, 0.F, 0.F));
                    render->setClipRectEnabled(true);
                    render->setClipRect(bbox * devicePixelRatio);
                    std::set<otime::RationalTime> videoDataDelete;
                    for (const auto& videoData : _videoData)
                    {
                        videoDataDelete.insert(videoData.first);
                    }
                    for (int x = _itemData.margin;
                        x < g.w() - _itemData.margin * 2;
                        x += _thumbnailWidth)
                    {
                        const math::BBox2i bbox(
                            g.min.x +
                            x,
                            g.min.y +
                            _itemData.margin +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing +
                            _itemData.fontMetrics.lineHeight +
                            _itemData.spacing,
                            _thumbnailWidth,
                            _thumbnailHeight);
                        if (bbox.intersects(v))
                        {
                            const otime::RationalTime time(
                                _timeRange.start_time().value() +
                                x / static_cast<double>(g.w() - _itemData.margin * 2) *
                                _timeRange.duration().value(),
                                _timeRange.duration().rate());
                            auto i = _videoData.find(time);
                            if (i != _videoData.end())
                            {
                                render->drawVideo(
                                    { i->second },
                                    { bbox * devicePixelRatio });
                                videoDataDelete.erase(time);
                            }
                            else
                            {
                                _videoDataFutures.push_back(_timeline->getVideo(time));
                            }
                        }
                    }
                    for (auto i : videoDataDelete)
                    {
                        const auto j = _videoData.find(i);
                        if (j != _videoData.end())
                        {
                            _videoData.erase(j);
                        }
                    }
                    render->setClipRectEnabled(false);
                }
            }

            void TimelineItem::tick()
            {
                auto i = _videoDataFutures.begin();
                while (i != _videoDataFutures.end())
                {
                    if (i->valid() &&
                        i->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        _doRender = true;
                        const auto videoData = i->get();
                        _videoData[videoData.time] = videoData;
                        i = _videoDataFutures.erase(i);
                        continue;
                    }
                    ++i;
                }
            }

            std::string TimelineItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    name :
                    std::string("Timeline");
            }
        }
    }
}
