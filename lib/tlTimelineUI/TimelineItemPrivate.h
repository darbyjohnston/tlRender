// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineUI/TimelineItem.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <ftk/UI/ToolButton.h>
#include <ftk/UI/Label.h>

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
            bool stopOnScrub = true;
            std::shared_ptr<ftk::ObservableValue<bool> > scrub;
            std::shared_ptr<ftk::ObservableValue<OTIO_NS::RationalTime> > timeScrub;
            std::vector<int> frameMarkers;
            std::shared_ptr<ThumbnailGenerator> thumbnailGenerator;

            struct Track
            {
                int index = 0;
                TrackType type = TrackType::None;
                OTIO_NS::TimeRange timeRange;
                std::shared_ptr<ftk::Label> label;
                std::shared_ptr<ftk::Label> durationLabel;
                std::vector<std::shared_ptr<IItem> > items;
                ftk::Size2I size;
                ftk::Box2I geom;
                int clipHeight = 0;
            };
            std::vector<Track> tracks;
            int firstVideoTrack = -1;
            int firstAudioTrack = -1;

            struct SizeData
            {
                std::optional<float> displayScale;
                int margin = 0;
                int spacing = 0;
                int border = 0;
                int handle = 0;
                ftk::FontInfo fontInfo = ftk::FontInfo("", 0);
                ftk::FontMetrics fontMetrics;
                ftk::Box2I scrollArea;
            };
            SizeData size;

            enum class MouseMode
            {
                None,
                CurrentTime
            };
            MouseMode mouseMode = MouseMode::None;

            std::shared_ptr<ftk::ValueObserver<OTIO_NS::RationalTime> > currentTimeObserver;
            std::shared_ptr<ftk::ValueObserver<OTIO_NS::TimeRange> > inOutRangeObserver;
            std::shared_ptr<ftk::ValueObserver<timeline::PlayerCacheInfo> > cacheInfoObserver;
        };
    }
}
