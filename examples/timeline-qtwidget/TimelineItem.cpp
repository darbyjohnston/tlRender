// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TimelineItem.h"

#include "TrackItem.h"

#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void TimelineItem::_init(
                const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("TimelineItem", itemData, context, parent);

                _timeline = timeline;
                _timeRange = timeline::getTimeRange(timeline);

                setBackgroundRole(ui::ColorRole::Window);

                for (const auto& child : timeline->tracks()->children())
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

                _timelineSize = observer::Value<math::Vector2i>::create();
            }

            std::shared_ptr<TimelineItem> TimelineItem::create(
                const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
                const ItemData& itemData,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
                out->_init(timeline, itemData, context, parent);
                return out;
            }

            TimelineItem::~TimelineItem()
            {
                //_cancelVideoRequests();
            }

            std::shared_ptr<observer::IValue<math::Vector2i> > TimelineItem::observeTimelineSize() const
            {
                return _timelineSize;
            }

            void TimelineItem::setCurrentTime(const otime::RationalTime& value)
            {
                if (time::compareExact(value, _currentTime))
                    return;
                _currentTime = value;
                _updates |= ui::Update::Draw;
                _updates |= ui::Update::Draw;
            }

            void TimelineItem::setScale(float value)
            {
                IItem::setScale(value);
                //if (_updates & ui::Update::Size)
                //{
                //    _cancelVideoRequests();
                //}
            }

            void TimelineItem::setThumbnailHeight(int value)
            {
                IItem::setThumbnailHeight(value);
                //if (_updates & ui::Update::Size)
                //{
                //    _cancelVideoRequests();
                //}
            }

            void TimelineItem::setViewport(const math::BBox2i& value)
            {
                IItem::setViewport(value);
                //if (_updates & ui::Update::Size)
                //{
                //    _cancelVideoRequests();
                //}
            }

            void TimelineItem::setGeometry(const math::BBox2i& value)
            {
                IWidget::setGeometry(value);

                float y =
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing;
                    //_thumbnailHeight;
                for (const auto& child : _children)
                {
                    const auto& sizeHint = child->getSizeHint();
                    child->setGeometry(math::BBox2i(
                        _geometry.min.x + _margin,
                        _geometry.min.y + y,
                        sizeHint.x,
                        sizeHint.y));
                    y += sizeHint.y;// +_spacing;
                }
            }

            /*void TimelineItem::tickEvent(const ui::TickEvent& event)
            {
                auto i = _videoDataFutures.begin();
                while (i != _videoDataFutures.end())
                {
                    if (i->second.valid() &&
                        i->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        const auto videoData = i->second.get();
                        _videoData[videoData.time] = videoData;
                        i = _videoDataFutures.erase(i);
                        _updates |= ui::Update::Draw;
                        continue;
                    }
                    ++i;
                }
            }*/

            void TimelineItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::Margin) * event.contentScale;
                _spacing = event.style->getSizeRole(ui::SizeRole::Spacing)* event.contentScale;
                _fontMetrics = event.fontSystem->getMetrics(imaging::FontInfo());

                //const auto& info = _timeline->getIOInfo();
                //_thumbnailWidth = !info.video.empty() ?
                //    static_cast<int>(_thumbnailHeight * info.video[0].size.getAspect()) :
                //    0;

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
                    _margin +
                    _timeRange.duration().rescaled_to(1.0).value() * _scale +
                    _margin,
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _thumbnailHeight +
                    childrenHeight +
                    _margin);

                _timelineSize->setIfChanged(_sizeHint);
            }

            void TimelineItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);
                _drawCurrentTime(event);
                _drawTimeTicks(event);
                //_drawThumbnails(event);
            }

            void TimelineItem::_drawCurrentTime(const ui::DrawEvent& event)
            {
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;

                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                if (!time::compareExact(_currentTime, time::invalidTime))
                {
                    math::Vector2i pos(
                        g.min.x +
                        _margin +
                        _currentTime.value() / _timeRange.duration().value() * (_sizeHint.x - _margin * 2),
                        g.min.y +
                        _margin);

                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(math::Vector2f(pos.x - _fontMetrics.lineHeight / 2, pos.y));
                    mesh.v.push_back(math::Vector2f(pos.x + _fontMetrics.lineHeight / 2, pos.y));
                    mesh.v.push_back(math::Vector2f(pos.x, pos.y + _fontMetrics.lineHeight));
                    mesh.triangles.push_back(geom::Triangle2({ 1, 2, 3 }));
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(ui::ColorRole::Text));

                    std::string label = _timeLabel(_currentTime, _timeUnits);
                    event.render->drawText(
                        event.fontSystem->getGlyphs(label, fontInfo),
                        math::Vector2i(
                            pos.x + _fontMetrics.lineHeight / 2 + _spacing,
                            pos.y + _fontMetrics.ascender),
                        event.style->getColorRole(ui::ColorRole::Text));
                }
            }

            void TimelineItem::_drawTimeTicks(const ui::DrawEvent& event)
            {
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;

                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                const float frameTick0 = _timeRange.start_time().value() /
                    _timeRange.duration().value() * (_sizeHint.x - _margin * 2);
                const float frameTick1 = (_timeRange.start_time().value() + 1.0) /
                    _timeRange.duration().value() * (_sizeHint.x - _margin * 2);
                const int frameWidth = frameTick1 - frameTick0;
                if (frameWidth >= 5)
                {
                    geom::TriangleMesh2 mesh;
                    size_t i = 1;
                    for (double t = 0.0; t < _timeRange.duration().value(); t += 1.0)
                    {
                        math::BBox2i bbox(
                            _geometry.min.x +
                            _margin + t / _timeRange.duration().value() * (_sizeHint.x - _margin * 2),
                            _geometry.min.y +
                            _margin +
                            _fontMetrics.lineHeight +
                            _spacing +
                            _fontMetrics.lineHeight +
                            _spacing,
                            1,
                            _fontMetrics.lineHeight);
                        if (bbox.intersects(_viewport))
                        {
                            bbox.min = bbox.min - _viewport.min;
                            bbox.max = bbox.max - _viewport.min;
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
                            imaging::Color4f(.6F, .6F, .6F));
                    }
                }

                const float secondsTick0 = _timeRange.start_time().value() /
                    (_timeRange.duration().value() / _timeRange.duration().rate()) * (_sizeHint.x - _margin * 2);
                const float secondsTick1 = (_timeRange.start_time().value() + 1.0) /
                    (_timeRange.duration().value() / _timeRange.duration().rate()) * (_sizeHint.x - _margin * 2);
                const int secondsWidth = secondsTick1 - secondsTick0;
                if (secondsWidth >= 5)
                {
                    std::string labelMax = _timeLabel(_timeRange.end_time_inclusive(), _timeUnits);
                    math::Vector2i labelMaxSize = event.fontSystem->measure(labelMax, fontInfo);
                    if (labelMaxSize.x < (secondsWidth - _spacing))
                    {
                        for (double t = 0.0;
                            t < _timeRange.duration().value();
                            t += _timeRange.duration().rate())
                        {
                            math::BBox2i bbox(
                                _geometry.min.x +
                                _margin +
                                t / _timeRange.duration().value() * (_sizeHint.x - _margin * 2),
                                _geometry.min.y +
                                _margin +
                                _fontMetrics.lineHeight +
                                _spacing,
                                labelMaxSize.x,
                                _fontMetrics.lineHeight);
                            if (bbox.intersects(_viewport))
                            {
                                bbox.min = bbox.min - _viewport.min;
                                bbox.max = bbox.max - _viewport.min;
                                std::string label = _timeLabel(
                                    otime::RationalTime(t, _timeRange.duration().rate()),
                                    _timeUnits);
                                event.render->drawText(
                                    event.fontSystem->getGlyphs(label, fontInfo),
                                    math::Vector2i(
                                        bbox.min.x,
                                        bbox.min.y +
                                        _fontMetrics.ascender),
                                    event.style->getColorRole(ui::ColorRole::Text));
                            }
                        }
                    }

                    geom::TriangleMesh2 mesh;
                    size_t i = 1;
                    for (double t = 0.0;
                        t < _timeRange.duration().value();
                        t += _timeRange.duration().rate())
                    {
                        math::BBox2i bbox(
                            _geometry.min.x +
                            _margin + t / _timeRange.duration().value() * (_sizeHint.x - _margin * 2),
                            _geometry.min.y +
                            _margin +
                            _fontMetrics.lineHeight +
                            _spacing +
                            _fontMetrics.lineHeight +
                            _spacing,
                            2,
                            _fontMetrics.lineHeight);
                        if (bbox.intersects(_viewport))
                        {
                            bbox.min = bbox.min - _viewport.min;
                            bbox.max = bbox.max - _viewport.min;
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
                            imaging::Color4f(.8F, .8F, .8F));
                    }
                }
            }

            /*void TimelineItem::_drawThumbnails(const ui::DrawEvent& event)
            {
                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                const math::BBox2i bbox(
                    g.min.x +
                    _margin,
                    g.min.y +
                    _margin +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing,
                    _sizeHint.x - _margin * 2,
                    _thumbnailHeight);
                event.render->drawRect(
                    bbox,
                    imaging::Color4f(0.F, 0.F, 0.F));
                event.render->setClipRectEnabled(true);
                event.render->setClipRect(bbox);

                std::set<otime::RationalTime> videoDataDelete;
                for (const auto& videoData : _videoData)
                {
                    videoDataDelete.insert(videoData.first);
                }

                for (int x = _margin; x < _sizeHint.x - _margin; x += _thumbnailWidth)
                {
                    math::BBox2i bbox(
                        _geometry.min.x +
                        x,
                        _geometry.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.lineHeight +
                        _spacing,
                        _thumbnailWidth,
                        _thumbnailHeight);
                    if (bbox.intersects(_viewport))
                    {
                        const int w = _sizeHint.x - _margin * 2;
                        const otime::RationalTime time = time::round(otime::RationalTime(
                            _timeRange.start_time().value() +
                            (w > 0 ? ((x - _margin) / static_cast<double>(w)) : 0) *
                            _timeRange.duration().value(),
                            _timeRange.duration().rate()));
                        auto i = _videoData.find(time);
                        if (i != _videoData.end())
                        {
                            bbox.min = bbox.min - _viewport.min;
                            bbox.max = bbox.max - _viewport.min;
                            event.render->drawVideo(
                                { i->second },
                                { bbox });
                            videoDataDelete.erase(time);
                        }
                        else
                        {
                            const auto j = _videoDataFutures.find(time);
                            if (j == _videoDataFutures.end())
                            {
                                _videoDataFutures[time] = _timeline->getVideo(time);
                            }
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

                event.render->setClipRectEnabled(false);
            }*/
            
            /*void TimelineItem::_cancelVideoRequests()
            {
                _timeline->cancelRequests();
                _videoDataFutures.clear();
            }*/
        }
    }
}
