// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <opentimelineio/clip.h>
#include <opentimelineio/track.h>

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
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<VideoClipItem> create(
                    const otio::Clip*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~VideoClipItem() override;

                void setOptions(const ItemOptions&) override;
                void setViewport(const math::BBox2i&) override;

                void tickEvent(const ui::TickEvent&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                void _textUpdate();

                void _drawInfo(const ui::DrawEvent&);
                void _drawThumbnails(const ui::DrawEvent&);

                void _cancelVideoRequests();

                const otio::Clip* _clip = nullptr;
                const otio::Track* _track = nullptr;
                file::Path _path;
                std::vector<file::MemoryRead> _memoryRead;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::string _label;
                std::string _durationLabel;
                ui::FontRole _fontRole = ui::FontRole::Label;
                int _margin = 0;
                int _spacing = 0;
                int _thumbnailWidth = 0;
                std::shared_ptr<io::IRead> _reader;
                std::future<io::Info> _ioInfoFuture;
                io::Info _ioInfo;
                std::map<otime::RationalTime, std::future<io::VideoData> > _videoDataFutures;
                std::map<otime::RationalTime, io::VideoData> _videoData;
            };
        }
    }
}
