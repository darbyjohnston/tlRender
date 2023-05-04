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

            TimelineTrackItem();

        public:
            ~TimelineTrackItem() override;

            static std::shared_ptr<TimelineTrackItem> create(
                const otio::Track*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::BBox2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
