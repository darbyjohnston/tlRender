// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <feather-tk/ui/ToolButton.h>
#include <feather-tk/ui/Label.h>

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
            std::shared_ptr<feather_tk::ObservableValue<bool> > scrub;
            std::shared_ptr<feather_tk::ObservableValue<OTIO_NS::RationalTime> > timeScrub;
            std::vector<int> frameMarkers;
            int minimumHeight = 0;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct Track
            {
                int index = 0;
                TrackType type = TrackType::None;
                OTIO_NS::TimeRange timeRange;
                std::shared_ptr<feather_tk::ToolButton> enabledButton;
                std::shared_ptr<feather_tk::Label> label;
                std::shared_ptr<feather_tk::Label> durationLabel;
                std::vector<std::shared_ptr<IItem> > items;
                feather_tk::Size2I size;
                feather_tk::Box2I geom;
                int clipHeight = 0;
            };
            std::vector<Track> tracks;

            struct SizeData
            {
                std::optional<float> displayScale;
                int margin = 0;
                int spacing = 0;
                int border = 0;
                int handle = 0;
                feather_tk::FontInfo fontInfo = feather_tk::FontInfo("", 0);
                feather_tk::FontMetrics fontMetrics;
                feather_tk::Box2I scrollArea;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<feather_tk::Box2I> dropTargets;
            };
            std::optional<DrawData> draw;

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
                feather_tk::Box2I geometry;
            };
            struct MouseItemDropTarget
            {
                int index = -1;
                int track = -1;
                feather_tk::Box2I mouse;
                feather_tk::Box2I draw;
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::None;
                std::vector<std::shared_ptr<MouseItemData> > items;
                std::vector<MouseItemDropTarget> dropTargets;
                int currentDropTarget = -1;
            };
            MouseData mouse;

            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::TimeRange> > inOutRangeObserver;
            std::shared_ptr<feather_tk::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;

            std::shared_ptr<IItem> getAssociated(
                const std::shared_ptr<IItem>&,
                int& index,
                int& trackIndex) const;

            std::vector<MouseItemDropTarget> getDropTargets(
                const feather_tk::Box2I& geometry,
                int index,
                int track);
        };
    }
}
