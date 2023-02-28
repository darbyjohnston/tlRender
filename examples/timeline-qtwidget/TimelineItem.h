// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "BaseItem.h"

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Timeline item.
            class TimelineItem : public BaseItem
            {
            protected:
                void _init(
                    const std::shared_ptr<timeline::Timeline>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&);

            public:
                static std::shared_ptr<TimelineItem> create(
                    const std::shared_ptr<timeline::Timeline>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&);

                ~TimelineItem() override;

                void preLayout() override;
                void layout(const math::BBox2i&) override;
                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const math::BBox2i& viewport,
                    float devicePixelRatio) override;
                void tick() override;

            private:
                static std::string _nameLabel(const std::string&);

                std::shared_ptr<timeline::Timeline> _timeline;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                int _thumbnailWidth = 0;
                std::string _label;
                std::string _durationLabel;
                std::string _startLabel;
                std::string _endLabel;
                math::BBox2i _viewportTmp;
                std::vector<std::future<timeline::VideoData> > _videoDataFutures;
                std::map<otime::RationalTime, timeline::VideoData> _videoData;
            };
        }
    }
}
