// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/GapItem.h>

namespace tl
{
    namespace timelineui
    {
        void GapItem::_init(
            ui::ColorRole colorRole,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>& gap,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IBasicItem::_init(
                !gap->name().empty() ? gap->name() : "Gap",
                colorRole,
                "tl::timelineui::GapItem",
                gap.value,
                scale,
                options,
                displayOptions,
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
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Gap>& gap,
            double scale,
            const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GapItem>(new GapItem);
            out->_init(
                colorRole,
                gap,
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
