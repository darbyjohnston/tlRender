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
            //! Video clip item.
            class VideoClipItem : public IItem
            {
            protected:
                void _init(
                    const otio::Clip*,
                    const std::shared_ptr<timeline::Timeline>&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<VideoClipItem> create(
                    const otio::Clip*,
                    const std::shared_ptr<timeline::Timeline>&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~VideoClipItem() override;

                void setScale(float) override;
                void setThumbnailHeight(int) override;
                void setViewport(const math::BBox2i&) override;

                void tickEvent(const ui::TickEvent&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                static std::string _nameLabel(const std::string&);

                void _cancelVideoRequests();

                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::string _label;
                std::string _durationLabel;
                std::string _startLabel;
                std::string _endLabel;
                imaging::FontInfo _fontInfo;
                int _margin = 0;
                int _spacing = 0;
                int _border = 0;
                imaging::FontMetrics _fontMetrics;
                int _thumbnailWidth = 0;
                std::map<otime::RationalTime, std::future<timeline::VideoData> > _videoDataFutures;
                std::map<otime::RationalTime, timeline::VideoData> _videoData;
            };
        }
    }
}
