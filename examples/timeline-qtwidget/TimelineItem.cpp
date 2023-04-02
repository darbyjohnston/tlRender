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

                _currentTime = observer::Value<otime::RationalTime>::create(time::invalidTime);
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
            {}

            void TimelineItem::setCurrentTime(const otime::RationalTime& value)
            {
                const otime::RationalTime tmp = math::clamp(
                    value,
                    _timeRange.start_time(),
                    _timeRange.end_time_inclusive());
                if (_currentTime->setIfChanged(tmp))
                {
                    _updates |= ui::Update::Draw;
                }
            }

            std::shared_ptr<observer::IValue<otime::RationalTime> > TimelineItem::observeCurrentTime() const
            {
                return _currentTime;
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

            void TimelineItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::Margin) * event.contentScale;
                _spacing = event.style->getSizeRole(ui::SizeRole::SpacingSmall)* event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                _fontMetrics = event.fontSystem->getMetrics(fontInfo);

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
                    childrenHeight +
                    _margin);
            }

            void TimelineItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);
                _drawTimeTicks(event);
                _drawCurrentTime(event);
            }
            
            void TimelineItem::enterEvent()
            {}

            void TimelineItem::leaveEvent()
            {}

            void TimelineItem::mouseMoveEvent(ui::MouseMoveEvent& event)
            {
                event.accept = true;
                _mousePos = event.pos;
                if (_currentTimeDrag)
                {
                    setCurrentTime(_posToTime(_mousePos.x));
                }
            }

            void TimelineItem::mousePressEvent(ui::MouseClickEvent& event)
            {
                event.accept = true;
                _mousePress = true;
                _mousePressPos = _mousePos;
                const math::BBox2i bbox = _getCurrentTimeBBox();
                if (bbox.contains(_mousePos))
                {
                    _currentTimeDrag = true;
                    setCurrentTime(_posToTime(_mousePos.x));
                }
            }

            void TimelineItem::mouseReleaseEvent(ui::MouseClickEvent& event)
            {
                event.accept = true;
                _mousePress = false;
                _currentTimeDrag = false;
            }

            void TimelineItem::_drawTimeTicks(const ui::DrawEvent& event)
            {
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                const math::BBox2i vp(0, 0, _viewport.w(), _viewport.h());
                math::BBox2i g = _geometry;

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
                            g.min.x +
                            _margin +
                            t / _timeRange.duration().value() * (_sizeHint.x - _margin * 2),
                            g.min.y +
                            _margin +
                            _fontMetrics.lineHeight +
                            _spacing +
                            _fontMetrics.lineHeight / 2,
                            1,
                            _fontMetrics.lineHeight / 2);
                        if (bbox.intersects(vp))
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
                                _spacing +
                                _fontMetrics.lineHeight +
                                _spacing,
                                labelMaxSize.x,
                                _fontMetrics.lineHeight);
                            if (bbox.intersects(vp))
                            {
                                std::string label = _timeLabel(
                                    _timeRange.start_time() + otime::RationalTime(t, _timeRange.duration().rate()),
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
                            _spacing,
                            2,
                            _fontMetrics.lineHeight);
                        if (bbox.intersects(vp))
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
                            imaging::Color4f(.8F, .8F, .8F));
                    }
                }
            }

            void TimelineItem::_drawCurrentTime(const ui::DrawEvent& event)
            {
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                math::BBox2i g = _geometry;

                const otime::RationalTime& currentTime = _currentTime->get();
                if (!time::compareExact(currentTime, time::invalidTime))
                {
                    math::Vector2i pos(
                        _timeToPos(currentTime),
                        g.min.y +
                        _margin);

                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(math::Vector2f(
                        pos.x -
                        _fontMetrics.lineHeight / 3,
                        pos.y +
                        _fontMetrics.lineHeight +
                        _spacing));
                    mesh.v.push_back(math::Vector2f(
                        pos.x +
                        _fontMetrics.lineHeight / 3,
                        pos.y +
                        _fontMetrics.lineHeight +
                        _spacing));
                    mesh.v.push_back(math::Vector2f(
                        pos.x,
                        pos.y +
                        _fontMetrics.lineHeight +
                        _spacing +
                        _fontMetrics.lineHeight / 2));
                    mesh.triangles.push_back(geom::Triangle2({ 1, 2, 3 }));
                    event.render->drawMesh(
                        mesh,
                        event.style->getColorRole(ui::ColorRole::Text));

                    std::string label = _timeLabel(currentTime, _timeUnits);
                    event.render->drawText(
                        event.fontSystem->getGlyphs(label, fontInfo),
                        math::Vector2i(
                            pos.x,
                            pos.y +
                            _fontMetrics.ascender),
                        event.style->getColorRole(ui::ColorRole::Text));
                }
            }

            math::BBox2i TimelineItem::_getCurrentTimeBBox() const
            {
                return math::BBox2i(
                    _geometry.min.x + _margin,
                    _geometry.min.y + _margin,
                    _geometry.w() - _margin * 2,
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing +
                    _fontMetrics.lineHeight +
                    _spacing);
            }

            otime::RationalTime TimelineItem::_posToTime(float value) const
            {
                otime::RationalTime out = time::invalidTime;
                const math::BBox2i bbox = _getCurrentTimeBBox();
                if (bbox.w() > 0)
                {
                    const float v = (value - bbox.min.x) / static_cast<float>(bbox.w());
                    out = time::round(
                        _timeRange.start_time() +
                        otime::RationalTime(
                            _timeRange.duration().value() * v,
                            _timeRange.duration().rate()));
                }
                return out;
            }

            float TimelineItem::_timeToPos(const otime::RationalTime& value) const
            {
                float out = 0.F;
                const otime::RationalTime& currentTime = _currentTime->get();
                if (!time::compareExact(currentTime, time::invalidTime))
                {
                    const math::BBox2i bbox = _getCurrentTimeBBox();
                    out = bbox.min.x +
                        (currentTime.value() - _timeRange.start_time().value()) /
                        _timeRange.duration().value() *
                        bbox.w();
                }
                return out;
            }
        }
    }
}
