// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/Init.h>

#include <tlTimelineUI/ThumbnailSystem.h>

#include <tlTimeline/Init.h>

#include <dtk/ui/Init.h>
#include <dtk/ui/IconSystem.h>

namespace tl_resource
{
    extern std::vector<uint8_t> ColorControls;
    extern std::vector<uint8_t> ColorPicker;
    extern std::vector<uint8_t> CompareA;
    extern std::vector<uint8_t> CompareB;
    extern std::vector<uint8_t> CompareDifference;
    extern std::vector<uint8_t> CompareHorizontal;
    extern std::vector<uint8_t> CompareOverlay;
    extern std::vector<uint8_t> Compare;
    extern std::vector<uint8_t> CompareTile;
    extern std::vector<uint8_t> CompareVertical;
    extern std::vector<uint8_t> CompareWipe;
    extern std::vector<uint8_t> Devices;
    extern std::vector<uint8_t> Export;
    extern std::vector<uint8_t> FileOpenSeparateAudio;
    extern std::vector<uint8_t> Files;
    extern std::vector<uint8_t> FrameShuttle0;
    extern std::vector<uint8_t> FrameShuttle1;
    extern std::vector<uint8_t> FrameShuttle2;
    extern std::vector<uint8_t> FrameShuttle3;
    extern std::vector<uint8_t> FrameShuttle4;
    extern std::vector<uint8_t> FrameShuttle5;
    extern std::vector<uint8_t> FrameShuttle6;
    extern std::vector<uint8_t> FrameShuttle7;
    extern std::vector<uint8_t> Hidden;
    extern std::vector<uint8_t> Info;
    extern std::vector<uint8_t> Messages;
    extern std::vector<uint8_t> PlaybackShuttle0;
    extern std::vector<uint8_t> PlaybackShuttle1;
    extern std::vector<uint8_t> PlaybackShuttle2;
    extern std::vector<uint8_t> PlaybackShuttle3;
    extern std::vector<uint8_t> PlaybackShuttle4;
    extern std::vector<uint8_t> PlaybackShuttle5;
    extern std::vector<uint8_t> PlaybackShuttle6;
    extern std::vector<uint8_t> PlaybackShuttle7;
    extern std::vector<uint8_t> View;
    extern std::vector<uint8_t> Visible;
    extern std::vector<uint8_t> WindowSecondary;
    extern std::vector<uint8_t> tlRender;
}

namespace tl
{
    namespace timelineui
    {
        void init(const std::shared_ptr<dtk::Context>& context)
        {
            tl::timeline::init(context);
            dtk::uiInit(context);
            ThumbnailSystem::create(context);

            auto iconSystem = context->getSystem<dtk::IconSystem>();
            iconSystem->add("ColorControls", tl_resource::ColorControls);
            iconSystem->add("ColorPicker", tl_resource::ColorPicker);
            iconSystem->add("CompareA", tl_resource::CompareA);
            iconSystem->add("CompareB", tl_resource::CompareB);
            iconSystem->add("CompareDifference", tl_resource::CompareDifference);
            iconSystem->add("CompareHorizontal", tl_resource::CompareHorizontal);
            iconSystem->add("CompareOverlay", tl_resource::CompareOverlay);
            iconSystem->add("Compare", tl_resource::Compare);
            iconSystem->add("CompareTile", tl_resource::CompareTile);
            iconSystem->add("CompareVertical", tl_resource::CompareVertical);
            iconSystem->add("CompareWipe", tl_resource::CompareWipe);
            iconSystem->add("Devices", tl_resource::Devices);
            iconSystem->add("Export", tl_resource::Export);
            iconSystem->add("FileOpenSeparateAudio", tl_resource::FileOpenSeparateAudio);
            iconSystem->add("Files", tl_resource::Files);
            iconSystem->add("FrameShuttle0", tl_resource::FrameShuttle0);
            iconSystem->add("FrameShuttle1", tl_resource::FrameShuttle1);
            iconSystem->add("FrameShuttle2", tl_resource::FrameShuttle2);
            iconSystem->add("FrameShuttle3", tl_resource::FrameShuttle3);
            iconSystem->add("FrameShuttle4", tl_resource::FrameShuttle4);
            iconSystem->add("FrameShuttle5", tl_resource::FrameShuttle5);
            iconSystem->add("FrameShuttle6", tl_resource::FrameShuttle6);
            iconSystem->add("FrameShuttle7", tl_resource::FrameShuttle7);
            iconSystem->add("Hidden", tl_resource::Hidden);
            iconSystem->add("Info", tl_resource::Info);
            iconSystem->add("Messages", tl_resource::Messages);
            iconSystem->add("PlaybackShuttle0", tl_resource::PlaybackShuttle0);
            iconSystem->add("PlaybackShuttle1", tl_resource::PlaybackShuttle1);
            iconSystem->add("PlaybackShuttle2", tl_resource::PlaybackShuttle2);
            iconSystem->add("PlaybackShuttle3", tl_resource::PlaybackShuttle3);
            iconSystem->add("PlaybackShuttle4", tl_resource::PlaybackShuttle4);
            iconSystem->add("PlaybackShuttle5", tl_resource::PlaybackShuttle5);
            iconSystem->add("PlaybackShuttle6", tl_resource::PlaybackShuttle6);
            iconSystem->add("PlaybackShuttle7", tl_resource::PlaybackShuttle7);
            iconSystem->add("View", tl_resource::View);
            iconSystem->add("Visible", tl_resource::Visible);
            iconSystem->add("WindowSecondary", tl_resource::WindowSecondary);
            iconSystem->add("tlRender", tl_resource::tlRender);
        }
    }
}
