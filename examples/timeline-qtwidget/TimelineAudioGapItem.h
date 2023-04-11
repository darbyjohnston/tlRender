// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "ITimelineItem.h"

#include <opentimelineio/gap.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
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

            public:
                ~TimelineAudioGapItem() override;

                static std::shared_ptr<TimelineAudioGapItem> create(
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
            };
        }
    }
}
