// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TrackItem.h"

#include "AudioClipItem.h"
#include "VideoClipItem.h"
#include "GapItem.h"

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void TrackItem::_init(
                const otio::Track* track,
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("TrackItem", timeline, context, parent);

                if (otio::Track::Kind::video == track->kind())
                {
                    _trackType = TrackType::Video;
                }
                else if (otio::Track::Kind::audio == track->kind())
                {
                    _trackType = TrackType::Audio;
                }

                _timeRange = track->trimmed_range();

                for (const auto& child : track->children())
                {
                    if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                    {
                        std::shared_ptr<IItem> clipItem;
                        switch (_trackType)
                        {
                        case TrackType::Video:
                            clipItem = VideoClipItem::create(
                                clip,
                                timeline,
                                context,
                                shared_from_this());
                            break;
                        case TrackType::Audio:
                            clipItem = AudioClipItem::create(
                                clip,
                                timeline,
                                context,
                                shared_from_this());
                            break;
                        }
                        const auto timeRangeOpt = track->trimmed_range_of_child(clip);
                        if (timeRangeOpt.has_value())
                        {
                            _childTimeRanges[clipItem] = timeRangeOpt.value();
                        }
                    }
                    else if (auto gap = dynamic_cast<otio::Gap*>(child.value))
                    {
                        auto gapItem = GapItem::create(
                            gap,
                            timeline,
                            context,
                            shared_from_this());
                        const auto timeRangeOpt = track->trimmed_range_of_child(gap);
                        if (timeRangeOpt.has_value())
                        {
                            _childTimeRanges[gapItem] = timeRangeOpt.value();
                        }
                    }
                }

                _label = _nameLabel(track->kind(), track->name());
                _durationLabel = IItem::_durationLabel(_timeRange.duration());
            }

            std::shared_ptr<TrackItem> TrackItem::create(
                const otio::Track* track,
                const std::shared_ptr<timeline::Timeline>& timeline,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<TrackItem>(new TrackItem);
                out->_init(track, timeline, context, parent);
                return out;
            }

            TrackItem::~TrackItem()
            {}

            void TrackItem::setGeometry(const math::BBox2i& value)
            {
                IItem::setGeometry(value);
                for (auto child : _children)
                {
                    if (auto item = std::dynamic_pointer_cast<IItem>(child))
                    {
                        const auto i = _childTimeRanges.find(item);
                        if (i != _childTimeRanges.end())
                        {
                            const math::Vector2i& sizeHint = child->getSizeHint();
                            math::BBox2i bbox(
                                _geometry.min.x +
                                i->second.start_time().rescaled_to(1.0).value() * _scale,
                                _geometry.min.y +
                                _margin +
                                _fontMetrics.lineHeight +
                                _margin,
                                sizeHint.x,
                                sizeHint.y);
                            child->setGeometry(bbox);
                        }
                    }
                }
            }

            void TrackItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                _margin = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                _fontMetrics = event.fontSystem->getMetrics(fontInfo);

                int childrenHeight = 0;
                for (const auto& child : _children)
                {
                    childrenHeight = std::max(childrenHeight, child->getSizeHint().y);
                }

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    _margin +
                    _fontMetrics.lineHeight +
                    _margin +
                    childrenHeight);
            }

            void TrackItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);

                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;

                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                //event.render->drawRect(
                //    g,
                //    event.style->getColorRole(ui::ColorRole::Red));

                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        _margin,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                math::Vector2i textSize = event.fontSystem->measure(_durationLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_durationLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        _margin -
                        textSize.x,
                        g.min.y +
                        _margin +
                        _fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
            }

            std::string TrackItem::_nameLabel(
                const std::string& kind,
                const std::string& name)
            {
                return !name.empty() && name != "Track" ?
                    string::Format("{0} Track: {1}").arg(kind).arg(name) :
                    string::Format("{0} Track").arg(kind);
            }
        }
    }
}
