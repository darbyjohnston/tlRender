// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ITimelineItem.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ui
    {
        TLRENDER_ENUM_IMPL(
            TimelineTimeUnits,
            "Seconds",
            "Frames",
            "Timecode");
        TLRENDER_ENUM_SERIALIZE_IMPL(TimelineTimeUnits);

        bool TimelineItemOptions::operator == (const TimelineItemOptions& other) const
        {
            return
                timeUnits == other.timeUnits &&
                scale == other.scale &&
                clipRectScale == other.clipRectScale &&
                thumbnails == other.thumbnails &&
                thumbnailHeight == other.thumbnailHeight &&
                waveformHeight == other.waveformHeight &&
                thumbnailFade == other.thumbnailFade;
        }

        bool TimelineItemOptions::operator != (const TimelineItemOptions& other) const
        {
            return !(*this == other);
        }

        void ITimelineItem::_init(
            const std::string& name,
            const TimelineItemData& data,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            _data = data;
        }

        ITimelineItem::ITimelineItem()
        {}

        ITimelineItem::~ITimelineItem()
        {}

        void ITimelineItem::setOptions(const TimelineItemOptions& value)
        {
            if (value == _options)
                return;
            _options = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        math::BBox2i ITimelineItem::_getClipRect(
            const math::BBox2i& value,
            float scale)
        {
            math::BBox2i out;
            const math::Vector2i c = value.getCenter();
            out.min.x = (value.min.x - c.x) * scale + c.x;
            out.min.y = (value.min.y - c.y) * scale + c.y;
            out.max.x = (value.max.x - c.x) * scale + c.x;
            out.max.y = (value.max.y - c.y) * scale + c.y;
            return out;
        }

        std::string ITimelineItem::_durationLabel(
            const otime::RationalTime& value,
            TimelineTimeUnits timeUnits)
        {
            std::string out;
            if (!time::compareExact(value, time::invalidTime))
            {
                switch (timeUnits)
                {
                case TimelineTimeUnits::Seconds:
                    out = string::Format("{0} @ {1}").
                        arg(value.rescaled_to(1.0).value(), 2).
                        arg(value.rate());
                    break;
                case TimelineTimeUnits::Frames:
                    out = string::Format("{0} @ {1}").
                        arg(value.value()).
                        arg(value.rate());
                    break;
                case TimelineTimeUnits::Timecode:
                    out = string::Format("{0} @ {1}").
                        arg(value.to_timecode()).
                        arg(value.rate());
                    break;
                }
            }
            return out;
        }

        std::string ITimelineItem::_timeLabel(
            const otime::RationalTime& value,
            TimelineTimeUnits timeUnits)
        {
            std::string out;
            if (!time::compareExact(value, time::invalidTime))
            {
                switch (timeUnits)
                {
                case TimelineTimeUnits::Seconds:
                    out = string::Format("{0}").arg(value.rescaled_to(1.0).value(), 2);
                    break;
                case TimelineTimeUnits::Frames:
                    out = string::Format("{0}").arg(value.value());
                    break;
                case TimelineTimeUnits::Timecode:
                    out = string::Format("{0}").arg(value.to_timecode());
                    break;
                }
            }
            return out;
        }
    }
}