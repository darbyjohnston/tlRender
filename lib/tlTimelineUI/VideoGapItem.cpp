// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/VideoGapItem.h>

namespace tl
{
    namespace timelineui
    {
        void VideoGapItem::_init(
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            const auto rangeOpt = gap->trimmed_range_in_parent();
            IBasicItem::_init(
                rangeOpt.has_value() ? rangeOpt.value() : time::invalidTimeRange,
                !gap->name().empty() ? gap->name() : "Gap",
                ui::ColorRole::VideoGap,
                getMarkers(gap),
                "tl::timelineui::VideoGapItem",
                itemData,
                context,
                parent);
        }

        VideoGapItem::VideoGapItem()
        {}

        VideoGapItem::~VideoGapItem()
        {}

        std::shared_ptr<VideoGapItem> VideoGapItem::create(
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<VideoGapItem>(new VideoGapItem);
            out->_init(gap, itemData, context, parent);
            return out;
        }
    }
}
