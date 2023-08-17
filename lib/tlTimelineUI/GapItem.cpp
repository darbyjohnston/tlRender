// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/GapItem.h>

namespace tl
{
    namespace timelineui
    {
        void GapItem::_init(
            ui::ColorRole colorRole,
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IBasicItem::_init(
                !gap->name().empty() ? gap->name() : "Gap",
                colorRole,
                "tl::timelineui::GapItem",
                gap.value,
                itemData,
                context,
                parent);
        }

        GapItem::GapItem()
        {}

        GapItem::~GapItem()
        {}

        std::shared_ptr<GapItem> GapItem::create(
            ui::ColorRole colorRole,
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GapItem>(new GapItem);
            out->_init(colorRole, gap, itemData, context, parent);
            return out;
        }
    }
}
