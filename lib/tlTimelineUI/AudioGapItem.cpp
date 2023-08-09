// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/AudioGapItem.h>

namespace tl
{
    namespace timelineui
    {
        void AudioGapItem::_init(
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            const otime::TimeRange& timeRange,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IBasicItem::_init(
                !gap->name().empty() ? gap->name() : "Gap",
                ui::ColorRole::AudioGap,
                getMarkers(gap),
                "tl::timelineui::AudioGapItem",
                timeRange,
                itemData,
                context,
                parent);
        }

        AudioGapItem::AudioGapItem()
        {}

        AudioGapItem::~AudioGapItem()
        {}

        std::shared_ptr<AudioGapItem> AudioGapItem::create(
            const otio::SerializableObject::Retainer<otio::Gap>& gap,
            const otime::TimeRange& timeRange,
            const ItemData& itemData,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioGapItem>(new AudioGapItem);
            out->_init(gap, timeRange, itemData, context, parent);
            return out;
        }
    }
}
