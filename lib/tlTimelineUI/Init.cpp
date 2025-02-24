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
    extern std::vector<uint8_t> Color;
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
    extern std::vector<uint8_t> FileOpenSeparateAudio;
    extern std::vector<uint8_t> Files;
    extern std::vector<uint8_t> Hidden;
    extern std::vector<uint8_t> Info;
    extern std::vector<uint8_t> Messages;
    extern std::vector<uint8_t> tlRender;
    extern std::vector<uint8_t> View;
    extern std::vector<uint8_t> Visible;
    extern std::vector<uint8_t> WindowFullScreen;
    extern std::vector<uint8_t> WindowSecondary;
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
            iconSystem->add("Color", tl_resource::Color);
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
            iconSystem->add("FileOpenSeparateAudio", tl_resource::FileOpenSeparateAudio);
            iconSystem->add("Files", tl_resource::Files);
            iconSystem->add("Hidden", tl_resource::Hidden);
            iconSystem->add("Info", tl_resource::Info);
            iconSystem->add("Messages", tl_resource::Messages);
            iconSystem->add("tlRender", tl_resource::tlRender);
            iconSystem->add("View", tl_resource::View);
            iconSystem->add("Visible", tl_resource::Visible);
            iconSystem->add("WindowFullScreen", tl_resource::WindowFullScreen);
            iconSystem->add("WindowSecondary", tl_resource::WindowSecondary);
        }
    }
}
