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

        public:
            ~TimelineVideoGapItem() override;

            static std::shared_ptr<TimelineVideoGapItem> create(
                const otio::Gap*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setOptions(const TimelineItemOptions&) override;

            void sizeEvent(const ui::SizeEvent&) override;
            void drawEvent(const ui::DrawEvent&) override;

        private:
            void _textUpdate();

            static std::string _nameLabel(const std::string&);

            otime::TimeRange _timeRange = time::invalidTimeRange;
            std::string _label;
            std::string _durationLabel;
            ui::FontRole _fontRole = ui::FontRole::Label;
            int _margin = 0;
            int _spacing = 0;
        };
    }
}
