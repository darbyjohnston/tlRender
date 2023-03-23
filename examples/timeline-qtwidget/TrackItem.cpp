// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "TrackItem.h"

//#include "ClipItem.h"
//#include "GapItem.h"

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            void TrackItem::_init(
                const otio::Track* track,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IItem::_init("TrackItem", context, parent);

                _timeRange = track->trimmed_range();

                /*for (const auto& child : track->children())
                {
                    if (auto clip = dynamic_cast<otio::Clip*>(child.value))
                    {
                        auto clipItem = ClipItem::create(clip, _itemData, context);
                        _children.push_back(clipItem);
                        const auto timeRangeOpt = track->trimmed_range_of_child(clip);
                        if (timeRangeOpt.has_value())
                        {
                            _timeRanges[clipItem] = timeRangeOpt.value();
                        }
                    }
                    else if (auto gap = dynamic_cast<otio::Gap*>(child.value))
                    {
                        auto gapItem = GapItem::create(gap, _itemData, context);
                        _children.push_back(gapItem);
                        const auto timeRangeOpt = track->trimmed_range_of_child(gap);
                        if (timeRangeOpt.has_value())
                        {
                            _timeRanges[gapItem] = timeRangeOpt.value();
                        }
                    }
                }*/

                _label = _nameLabel(track->kind(), track->name());
                _durationLabel = IItem::_durationLabel(_timeRange.duration());
            }

            std::shared_ptr<TrackItem> TrackItem::create(
                const otio::Track* track,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<TrackItem>(new TrackItem);
                out->_init(track, context, parent);
                return out;
            }

            TrackItem::~TrackItem()
            {}

            void TrackItem::setGeometry(const math::BBox2i& value)
            {
                IItem::setGeometry(value);
            }

            void TrackItem::sizeEvent(const ui::SizeEvent& event)
            {
                IItem::sizeEvent(event);

                const int m = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                const auto fontMetrics = event.fontSystem->getMetrics(fontInfo);

                _sizeHint = math::Vector2i(
                    _timeRange.duration().rescaled_to(1.0).value() * _scale,
                    m +
                    fontMetrics.lineHeight +
                    m);
            }

            void TrackItem::drawEvent(const ui::DrawEvent& event)
            {
                IItem::drawEvent(event);

                const int m = event.style->getSizeRole(ui::SizeRole::MarginSmall) * event.contentScale;
                auto fontInfo = _fontInfo;
                fontInfo.size *= event.contentScale;
                const auto fontMetrics = event.fontSystem->getMetrics(fontInfo);

                math::BBox2i g = _geometry;
                g.min = g.min - _viewport.min;
                g.max = g.max - _viewport.min;

                event.render->drawRect(
                    g,
                    event.style->getColorRole(ui::ColorRole::Red));

                event.render->drawText(
                    event.fontSystem->getGlyphs(_label, fontInfo),
                    math::Vector2i(
                        g.min.x +
                        m,
                        g.min.y +
                        m +
                        fontMetrics.ascender),
                    event.style->getColorRole(ui::ColorRole::Text));
                math::Vector2i textSize = event.fontSystem->measure(_durationLabel, fontInfo);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_durationLabel, fontInfo),
                    math::Vector2i(
                        g.max.x -
                        m -
                        textSize.x,
                        g.min.y +
                        m +
                        fontMetrics.ascender),
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
