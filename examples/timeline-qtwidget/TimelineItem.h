// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <tlCore/ValueObserver.h>

#include <opentimelineio/timeline.h>

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
                    const otio::SerializableObject::Retainer<otio::Timeline>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<TimelineItem> create(
                    const otio::SerializableObject::Retainer<otio::Timeline>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~TimelineItem() override;

                std::shared_ptr<observer::IValue<math::Vector2i> > observeTimelineSize() const;

                void setCurrentTime(const otime::RationalTime&);

                void setScale(float) override;
                void setThumbnailHeight(int) override;
                void setViewport(const math::BBox2i&) override;

                void setGeometry(const math::BBox2i&) override;
                //void tickEvent(const ui::TickEvent&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                void _drawCurrentTime(const ui::DrawEvent&);
                void _drawTimeTicks(const ui::DrawEvent&);
                //void _drawThumbnails(const ui::DrawEvent&);

                //void _cancelVideoRequests();

                otio::SerializableObject::Retainer<otio::Timeline> _timeline;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                otime::RationalTime _currentTime = time::invalidTime;
                imaging::FontInfo _fontInfo;
                int _margin = 0;
                int _spacing = 0;
                imaging::FontMetrics _fontMetrics;
                //int _thumbnailWidth = 0;
                std::shared_ptr<observer::Value<math::Vector2i> > _timelineSize;
                //std::map<otime::RationalTime, std::future<timeline::VideoData> > _videoDataFutures;
                //std::map<otime::RationalTime, timeline::VideoData> _videoData;
            };
        }
    }
}
