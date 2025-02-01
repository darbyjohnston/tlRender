// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItem.h>

#include <tlUI/Label.h>
#include <tlUI/ThumbnailSystem.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineItem::Private
        {
            std::shared_ptr<timeline::Player> player;
            OTIO_NS::RationalTime currentTime = time::invalidTime;
            OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
            timeline::PlayerCacheInfo cacheInfo;
            bool editable = false;
            bool stopOnScrub = true;
            std::shared_ptr<dtk::ObservableValue<bool> > scrub;
            std::shared_ptr<dtk::ObservableValue<OTIO_NS::RationalTime> > timeScrub;
            std::vector<int> frameMarkers;
            int minimumHeight = 0;
            std::shared_ptr<ui::ThumbnailGenerator> thumbnailGenerator;

            struct Track
            {
                int index = 0;
                TrackType type = TrackType::None;
                OTIO_NS::TimeRange timeRange;
                std::shared_ptr<ui::ToolButton> enabledButton;
                std::shared_ptr<ui::Label> label;
                std::shared_ptr<ui::Label> durationLabel;
                std::vector<std::shared_ptr<IItem> > items;
                math::Size2i size;
                math::Box2i geom;
                int clipHeight = 0;
            };
            std::vector<Track> tracks;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int spacing = 0;
                int border = 0;
                int handle = 0;
                image::FontInfo fontInfo = image::FontInfo("", 0);
                image::FontMetrics fontMetrics;

                math::Vector2i scrollPos;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<math::Box2i> dropTargets;
            };
            DrawData draw;

            enum class MouseMode
            {
                None,
                CurrentTime,
                Item
            };
            struct MouseItemData
            {
                MouseItemData();
                MouseItemData(
                    const std::shared_ptr<IItem>&,
                    int index,
                    int track);

                ~MouseItemData();

                std::shared_ptr<IItem> p;
                int index = -1;
                int track = -1;
                math::Box2i geometry;
            };
            struct MouseItemDropTarget
            {
                int index = -1;
                int track = -1;
                math::Box2i mouse;
                math::Box2i draw;
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                std::vector<std::shared_ptr<MouseItemData> > items;
                std::vector<MouseItemDropTarget> dropTargets;
                int currentDropTarget = -1;
            };
            MouseData mouse;

            std::shared_ptr<dtk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<dtk::ValueObserver<OTIO_NS::TimeRange> > inOutRangeObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;

            std::shared_ptr<IItem> getAssociated(
                const std::shared_ptr<IItem>&,
                int& index,
                int& trackIndex) const;

            std::vector<MouseItemDropTarget> getDropTargets(
                const math::Box2i& geometry,
                int index,
                int track);
        };
    }
}
