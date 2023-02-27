// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <opentimelineio/track.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Track item.
            class TrackItem : public BaseItem
            {
            protected:
                void _init(
                    const otio::Track*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&);

            public:
                static std::shared_ptr<TrackItem> create(
                    const otio::Track*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&);

                ~TrackItem() override;

                void preLayout() override;
                void layout(const math::BBox2i&) override;
                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const math::BBox2i& viewport,
                    float devicePixelRatio) override;

            private:
                static std::string _nameLabel(
                    const std::string& kind,
                    const std::string& name);

                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::map<std::shared_ptr<BaseItem>, otime::TimeRange> _timeRanges;
                std::string _label;
                std::string _durationLabel;
            };
        }
    }
}
