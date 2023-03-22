// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <tlTimeline/Timeline.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Timeline item.
            class TimelineItem : public IItem
            {
            protected:
                void _init(
                    const std::shared_ptr<timeline::Timeline>&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<TimelineItem> create(
                    const std::shared_ptr<timeline::Timeline>&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~TimelineItem() override;

                void tickEvent(const ui::TickEvent&) override;
                void setGeometry(const math::BBox2i&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                static std::string _nameLabel(const std::string&);

                std::shared_ptr<timeline::Timeline> _timeline;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                math::Vector2i _timelineSize;
                int _thumbnailWidth = 0;
                std::string _label;
                std::string _durationLabel;
                std::string _startLabel;
                std::string _endLabel;
                //math::BBox2i _viewportTmp;
                std::vector<std::future<timeline::VideoData> > _videoDataFutures;
                std::map<otime::RationalTime, timeline::VideoData> _videoData;
            };
        }
    }
}
