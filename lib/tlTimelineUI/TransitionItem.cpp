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
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto rangeOpt = transition->trimmed_range_in_parent();
            IBasicItem::_init(
                rangeOpt.has_value() ? rangeOpt.value() : time::invalidTimeRange,
                !transition->name().empty() ? transition->name() : "Transition",
                ui::ColorRole::Transition,
                {},
                "tl::timelineui::TransitionItem",
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
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TransitionItem>(new TransitionItem);
            out->_init(transition, itemData, context, parent);
            return out;
        }
    }
}
