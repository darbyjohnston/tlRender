// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <opentimelineio/track.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Track item.
            class TrackItem : public IItem
            {
            protected:
                void _init(
                    const otio::Track*,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<TrackItem> create(
                    const otio::Track*,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~TrackItem() override;

                void setGeometry(const math::BBox2i&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                static std::string _nameLabel(
                    const std::string& kind,
                    const std::string& name);

                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::map<std::shared_ptr<IItem>, otime::TimeRange> _childTimeRanges;
                std::string _label;
                std::string _durationLabel;
                imaging::FontInfo _fontInfo;
                int _margin = 0;
                imaging::FontMetrics _fontMetrics;
            };
        }
    }
}
