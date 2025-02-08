// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <dtk/ui/ToolButton.h>
#include <dtk/ui/Label.h>

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
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct Track
            {
                int index = 0;
                TrackType type = TrackType::None;
                OTIO_NS::TimeRange timeRange;
                std::shared_ptr<dtk::ToolButton> enabledButton;
                std::shared_ptr<dtk::Label> label;
                std::shared_ptr<dtk::Label> durationLabel;
                std::vector<std::shared_ptr<IItem> > items;
                dtk::Size2I size;
                dtk::Box2I geom;
                int clipHeight = 0;
            };
            std::vector<Track> tracks;

            struct SizeData
            {
                bool init = true;
                float displayScale = 0.F;
                int margin = 0;
                int spacing = 0;
                int border = 0;
                int handle = 0;
                dtk::FontInfo fontInfo = dtk::FontInfo("", 0);
                dtk::FontMetrics fontMetrics;

                dtk::V2I scrollPos;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<dtk::Box2I> dropTargets;
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
                dtk::Box2I geometry;
            };
            struct MouseItemDropTarget
            {
                int index = -1;
                int track = -1;
                dtk::Box2I mouse;
                dtk::Box2I draw;
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
                const dtk::Box2I& geometry,
                int index,
                int track);
        };
    }
}
