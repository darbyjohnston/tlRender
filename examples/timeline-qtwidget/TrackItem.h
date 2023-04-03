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
            //! Track types.
            enum class TrackType
            {
                None,
                Video,
                Audio
            };

            //! Track item.
            class TrackItem : public IItem
            {
            protected:
                void _init(
                    const otio::Track*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<TrackItem> create(
                    const otio::Track*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~TrackItem() override;

                void setGeometry(const math::BBox2i&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                TrackType _trackType = TrackType::None;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::map<std::shared_ptr<IItem>, otime::TimeRange> _childTimeRanges;
                int _margin = 0;
            };
        }
    }
}
