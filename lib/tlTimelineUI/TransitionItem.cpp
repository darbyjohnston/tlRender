// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto timeRangeOpt = transition->trimmed_range_in_parent();
            IItem::_init(
                "tl::timelineui::TransitionItem",
                transition.value,
                timeRangeOpt.has_value() ? timeRangeOpt.value() : time::invalidTimeRange,
                scale,
                options,
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
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TransitionItem>(new TransitionItem);
            out->_init(transition, scale, options, itemData, context, parent);
            return out;
        }
    }
}
