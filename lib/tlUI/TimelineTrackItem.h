// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/ITimelineItem.h>

#include <opentimelineio/track.h>

namespace tl
{
    namespace ui
    {
        //! Track types.
        enum class TimelineTrackType
        {
            None,
            Video,
            Audio
        };

        //! Track item.
        class TimelineTrackItem : public ITimelineItem
        {
        protected:
            void _init(
                const otio::Track*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        public:
            ~TimelineTrackItem() override;

            static std::shared_ptr<TimelineTrackItem> create(
                const otio::Track*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::BBox2i&) override;
            void sizeEvent(const ui::SizeEvent&) override;
            void drawEvent(const ui::DrawEvent&) override;

        private:
            TimelineTrackType _trackType = TimelineTrackType::None;
            otime::TimeRange _timeRange = time::invalidTimeRange;
            std::map<std::shared_ptr<ITimelineItem>, otime::TimeRange> _childTimeRanges;
            int _margin = 0;
        };
    }
}
