// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/ITimelineItem.h>

#include <opentimelineio/gap.h>

namespace tl
{
    namespace ui
    {
        //! Timeline video gap item.
        class TimelineVideoGapItem : public ITimelineItem
        {
        protected:
            void _init(
                const otio::Gap*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineVideoGapItem();

        public:
            ~TimelineVideoGapItem() override;

            static std::shared_ptr<TimelineVideoGapItem> create(
                const otio::Gap*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setOptions(const TimelineItemOptions&) override;

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            void _textUpdate();

            static std::string _nameLabel(const std::string&);

            TLRENDER_PRIVATE();
        };
    }
}
