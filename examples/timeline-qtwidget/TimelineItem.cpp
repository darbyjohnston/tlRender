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
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("TimelineItem", context, parent);

                setBackgroundRole(ui::ColorRole::Window);

                _timeline = timeline;
                _timeRange = timeline->getTimeRange();

                const auto otioTimeline = timeline->getTimeline();
                for (const auto& child : otioTimeline->tracks()->children())
                {
                    if (const auto* track = dynamic_cast<otio::Track*>(child.value))
                    {
                        auto trackItem = TrackItem::create(track, context, shared_from_this());
                    }
                }

                _label = _nameLabel(otioTimeline->name());
                _durationLabel = IItem::_durationLabel(_timeRange.duration());
                _startLabel = _timeLabel(_timeRange.start_time());
                _endLabel = _timeLabel(_timeRange.end_time_inclusive());

                _timelineSize = observer::Value<math::Vector2i>::create();
            }

            std::shared_ptr<TimelineItem> TimelineItem::create(
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<TimelineItem>(new TimelineItem);
                out->_init(timeline, context, parent);
                return out;
            }

            TimelineItem::~TimelineItem()
            {
                _cancelVideoRequests();
            }

            std::shared_ptr<observer::IValue<math::Vector2i> > TimelineItem::observeTimelineSize() const
            {
                return _timelineSize;
            }

            void TimelineItem::setScale(float value)
            {
                IItem::setScale(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelVideoRequests();
                }
            }

            void TimelineItem::setThumbnailHeight(int value)
            {
                IItem::setThumbnailHeight(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelVideoRequests();
                }
            }

            void TimelineItem::setViewport(const math::BBox2i& value)
            {
                IItem::setViewport(value);
                if (_updates & ui::Update::Size)
                {
                    _cancelVideoRequests();
                }
            }

            void TimelineItem::tickEvent(const ui::TickEvent& event)
            {
                auto i = _videoDataFutures.begin();
                while (i != _videoDataFutures.end())
                {
                    if (i->valid() &&
                        i->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        const auto videoData = i->get();
                        _videoData[videoData.time] = videoData;
                        i = _videoDataFutures.erase(i);
                        _updates |= ui::Update::Draw;
                        continue;
                    }
                    ++i;
                }
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
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _thumbnailHeight;
                for (const auto& child : _children)
                {
                    const auto& sizeHint = child->getSizeHint();
                    child->setGeometry(math::BBox2i(
                        _geometry.min.x + _margin,
                        _geometry.min.y + y,
                        sizeHint.x,
                        sizeHint.y));
                    y += sizeHint.y;
                }
            }

            void TimelineItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::Margin) * event.contentScale;
                _spacing = event.style->getSizeRole(ui::SizeRole::Spacing) * event.contentScale;
                _fontMetrics = event.fontSystem->getMetrics(imaging::FontInfo());

                const auto& info = _timeline->getIOInfo();
                _thumbnailWidth = !info.video.empty() ?
                    static_cast<int>(_thumbnailHeight * info.video[0].size.getAspect()) :
                    0;

                int childrenHeight = 0;
                for (const auto& child : _children)
                {
                    childrenHeight += child->getSizeHint().y;
                }

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

                const auto& timelineSize = _timelineSize->get();
                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                event.render->drawText(
                    event.fontSystem->getGlyphs(_startLabel, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));

                math::Vector2i textSize = event.fontSystem->measure(_durationLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_durationLabel, fontInfo),
                    math::Vector2i(
                        g.min.x + _sizeHint.x -
                        _margin -
                        textSize.x,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                textSize = event.fontSystem->measure(_endLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_endLabel, fontInfo),
                    math::Vector2i(
                        g.min.x + _sizeHint.x -
                        _margin -
                        textSize.x,
                        g.min.y +
                        _margin +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));

                const float frameTick0 = _timeRange.start_time().value() /
                    _timeRange.duration().value() * (_sizeHint.x - _margin * 2);
                const float frameTick1 = (_timeRange.start_time().value() + 1.0) /
                    _timeRange.duration().value() * (_sizeHint.x - _margin * 2);
                const int frameWidth = frameTick1 - frameTick0;
                if (frameWidth >= 5)
                {
                    std::string labelMax = string::Format("{0}").arg(_timeRange.end_time_inclusive().value());
                    math::Vector2i labelMaxSize = event.fontSystem->measure(labelMax, fontInfo);
                    if (labelMaxSize.x < (frameWidth - _spacing))
                    {
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
                                labelMaxSize.x,
                                _fontMetrics.lineHeight);
                            if (bbox.intersects(_viewport))
                            {
                                bbox.min = bbox.min - _viewport.min;
                                bbox.max = bbox.max - _viewport.min;
                                std::string label = string::Format("{0}").arg(t);
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
                    std::string labelMax = string::Format("{0}").arg(_timeRange.end_time_inclusive().value());
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
                                _spacing +
                                _fontMetrics.lineHeight +
                                _spacing,
                                labelMaxSize.x,
                                _fontMetrics.lineHeight);
                            if (bbox.intersects(_viewport))
                            {
                                bbox.min = bbox.min - _viewport.min;
                                bbox.max = bbox.max - _viewport.min;
                                std::string label = string::Format("{0}").arg(t);
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
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing,
                    timelineSize.x - _margin * 2,
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
                        _spacing +
                        _fontMetrics.lineHeight +
                        _spacing,
                        _thumbnailWidth,
                        _thumbnailHeight);
                    if (bbox.intersects(_viewport))
                    {
                        const int w = _sizeHint.x - _margin * 2;
                        const otime::RationalTime time(
                            _timeRange.start_time().value() +
                            (w > 0 ? ((x - _margin) / static_cast<double>(w)) : 0) *
                            _timeRange.duration().value(),
                            _timeRange.duration().rate());
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
                event.render->setClipRectEnabled(false);
            }

            std::string TimelineItem::_nameLabel(const std::string& name)
            {
                return !name.empty() ?
                    name :
                    std::string("Timeline");
            }    
            
            void TimelineItem::_cancelVideoRequests()
            {
                _timeline->cancelRequests();
                _videoDataFutures.clear();
            }
        }
    }
}
