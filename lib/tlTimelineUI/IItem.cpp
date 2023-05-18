// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IItem.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace timelineui
    {
        bool ItemOptions::operator == (const ItemOptions& other) const
        {
            return
                timeUnits == other.timeUnits &&
                clipRectScale == other.clipRectScale &&
                thumbnails == other.thumbnails &&
                thumbnailHeight == other.thumbnailHeight &&
                waveformHeight == other.waveformHeight &&
                thumbnailFade == other.thumbnailFade;
        }

        bool ItemOptions::operator != (const ItemOptions& other) const
        {
            return !(*this == other);
        }

        void IItem::_init(
            const std::string& name,
            const ItemData& data,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            _data = data;
        }

        IItem::IItem()
        {}

        IItem::~IItem()
        {}

        void IItem::setScale(float value)
        {
            if (value == _scale)
                return;
            _scale = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void IItem::setOptions(const ItemOptions& value)
        {
            if (value == _options)
                return;
            _options = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        math::BBox2i IItem::_getClipRect(
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

        std::string IItem::_durationLabel(
            const otime::RationalTime& value,
            timeline::TimeUnits timeUnits)
        {
            std::string out;
            if (!time::compareExact(value, time::invalidTime))
            {
                switch (timeUnits)
                {
                case timeline::TimeUnits::Seconds:
                    out = string::Format("{0} {1}").
                        arg(value.rescaled_to(1.0).value(), 2).
                        arg(value.rate());
                    break;
                case timeline::TimeUnits::Frames:
                    out = string::Format("{0} {1}").
                        arg(value.value()).
                        arg(value.rate());
                    break;
                case timeline::TimeUnits::Timecode:
                    out = string::Format("{0} {1}").
                        arg(value.to_timecode()).
                        arg(value.rate());
                    break;
                }
            }
            return out;
        }

        std::string IItem::_timeLabel(
            const otime::RationalTime& value,
            timeline::TimeUnits timeUnits)
        {
            std::string out;
            if (!time::compareExact(value, time::invalidTime))
            {
                switch (timeUnits)
                {
                case timeline::TimeUnits::Seconds:
                    out = string::Format("{0}").arg(value.rescaled_to(1.0).value(), 2);
                    break;
                case timeline::TimeUnits::Frames:
                    out = string::Format("{0}").arg(value.value());
                    break;
                case timeline::TimeUnits::Timecode:
                    out = string::Format("{0}").arg(value.to_timecode());
                    break;
                }
            }
            return out;
        }
    }
}