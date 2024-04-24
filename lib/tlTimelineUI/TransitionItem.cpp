// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TransitionItem.h>

namespace tl
{
    namespace timelineui
    {
        void TransitionItem::_init(
            const otio::SerializableObject::Retainer<otio::Transition>& transition,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            otime::TimeRange timeRange = time::invalidTimeRange;
            otime::TimeRange trimmedRange = time::invalidTimeRange;
            const auto timeRangeOpt = transition->trimmed_range_in_parent();
            if (timeRangeOpt.has_value())
            {
                timeRange = timeRangeOpt.value();
                trimmedRange = otime::TimeRange(
                    otime::RationalTime(0.0, timeRange.duration().rate()),
                    timeRange.duration());
            }
            IItem::_init(
                "tl::timelineui::TransitionItem",
                timeRange,
                trimmedRange,
                scale,
                options,
                displayOptions,
                itemData,
                context,
                parent);
        }

        TransitionItem::TransitionItem()
        {}

        TransitionItem::~TransitionItem()
        {}

        std::shared_ptr<TransitionItem> TransitionItem::create(
            const otio::SerializableObject::Retainer<otio::Transition>& transition,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TransitionItem>(new TransitionItem);
            out->_init(
                transition,
                scale,
                options,
                displayOptions,
                itemData,
                context,
                parent);
            return out;
        }
    }
}
