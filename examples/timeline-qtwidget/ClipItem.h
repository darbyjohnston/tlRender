// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <opentimelineio/clip.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Clip item.
            class ClipItem : public IItem
            {
            protected:
                void _init(
                    const otio::Clip*,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<ClipItem> create(
                    const otio::Clip*,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~ClipItem() override;

                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                static std::string _nameLabel(const std::string&);

                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::string _label;
                std::string _durationLabel;
                std::string _startLabel;
                std::string _endLabel;
                imaging::FontInfo _fontInfo;
            };
        }
    }
}
