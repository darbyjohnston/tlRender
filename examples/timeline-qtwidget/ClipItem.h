// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

#include <opentimelineio/clip.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Clip item.
            class ClipItem : public BaseItem
            {
            protected:
                void _init(
                    const otio::Clip*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&);

            public:
                static std::shared_ptr<ClipItem> create(
                    const otio::Clip*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&);

                ~ClipItem() override;

                void preLayout() override;
                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const math::BBox2i& viewport,
                    float devicePixelRatio) override;

            private:
                static std::string _nameLabel(const std::string&);

                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::string _label;
                std::string _durationLabel;
                std::string _startLabel;
                std::string _endLabel;
            };
        }
    }
}
