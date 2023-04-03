// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "IItem.h"

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            TLRENDER_ENUM_IMPL(
                TimeUnits,
                "Seconds",
                "Frames",
                "Timecode");
            TLRENDER_ENUM_SERIALIZE_IMPL(TimeUnits);

            bool ItemOptions::operator == (const ItemOptions& other) const
            {
                return
                    timeUnits == other.timeUnits &&
                    scale == other.scale &&
                    thumbnailHeight == other.thumbnailHeight &&
                    waveformHeight == other.waveformHeight;
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

            void IItem::setOptions(const ItemOptions& value)
            {
                if (value == _options)
                    return;
                _options = value;
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }

            void IItem::setViewport(const math::BBox2i& value)
            {
                if (value == _viewport)
                    return;
                _viewport = value;
                _updates |= ui::Update::Size;
                _updates |= ui::Update::Draw;
            }
            
            bool IItem::_insideViewport() const
            {
                const math::BBox2i vp(0, 0, _viewport.w(), _viewport.h());
                return _geometry.intersects(vp);
            }

            std::string IItem::_durationLabel(const otime::RationalTime& value, TimeUnits timeUnits)
            {
                std::string out;
                if (!time::compareExact(value, time::invalidTime))
                {
                    switch (timeUnits)
                    {
                    case TimeUnits::Seconds:
                        out = string::Format("{0} @ {1}").
                            arg(value.rescaled_to(1.0).value(), 2).
                            arg(value.rate());
                        break;
                    case TimeUnits::Frames:
                        out = string::Format("{0} @ {1}").
                            arg(value.value()).
                            arg(value.rate());
                        break;
                    case TimeUnits::Timecode:
                        out = string::Format("{0} @ {1}").
                            arg(value.to_timecode()).
                            arg(value.rate());
                        break;
                    }
                }
                return out;
            }

            std::string IItem::_timeLabel(const otime::RationalTime& value, TimeUnits timeUnits)
            {
                std::string out;
                if (!time::compareExact(value, time::invalidTime))
                {
                    switch (timeUnits)
                    {
                    case TimeUnits::Seconds:
                        out = string::Format("{0}").arg(value.rescaled_to(1.0).value(), 2);
                        break;
                    case TimeUnits::Frames:
                        out = string::Format("{0}").arg(value.value());
                        break;
                    case TimeUnits::Timecode:
                        out = string::Format("{0}").arg(value.to_timecode());
                        break;
                    }
                }
                return out;
            }
        }
    }
}
