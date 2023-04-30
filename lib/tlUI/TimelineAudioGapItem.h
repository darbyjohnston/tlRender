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
        //! Timeline audio gap item.
        class TimelineAudioGapItem : public ITimelineItem
        {
        protected:
            void _init(
                const otio::Gap*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineAudioGapItem();

        public:
            ~TimelineAudioGapItem() override;

            static std::shared_ptr<TimelineAudioGapItem> create(
                const otio::Gap*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setOptions(const TimelineItemOptions&) override;

            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const DrawEvent&) override;

        private:
            void _textUpdate();

            static std::string _nameLabel(const std::string&);

            TLRENDER_PRIVATE();
        };
    }
}
