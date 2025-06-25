// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/GapItem.h>

namespace tl
{
    namespace timelineui
    {
        void GapItem::_init(
            const std::shared_ptr<feather_tk::Context>& context,
            feather_tk::ColorRole colorRole,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>& gap,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<IWidget>& parent)
        {
            IBasicItem::_init(
                context,
                !gap->name().empty() ? gap->name() : "Gap",
                colorRole,
                "tl::timelineui::GapItem",
                gap.value,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
        }

        GapItem::GapItem()
        {}

        GapItem::~GapItem()
        {}

        std::shared_ptr<GapItem> GapItem::create(
            const std::shared_ptr<feather_tk::Context>& context,
            feather_tk::ColorRole colorRole,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>& gap,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GapItem>(new GapItem);
            out->_init(
                context,
                colorRole,
                gap,
                scale,
                options,
                displayOptions,
                itemData,
                parent);
            return out;
        }
    }
}
