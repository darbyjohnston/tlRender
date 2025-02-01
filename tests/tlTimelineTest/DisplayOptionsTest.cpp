// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/DisplayOptionsTest.h>

#include <tlTimeline/DisplayOptions.h>

#include <dtk/core/Assert.h>
#include <dtk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        DisplayOptionsTest::DisplayOptionsTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::DisplayOptionsTest", context)
        {}

        std::shared_ptr<DisplayOptionsTest> DisplayOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<DisplayOptionsTest>(new DisplayOptionsTest(context));
        }

        void DisplayOptionsTest::run()
        {
            {
                _enum<Channels>("Channels", getChannelsEnums);
            }
            {
                Color color;
                color.enabled = true;
                DTK_ASSERT(color == color);
                DTK_ASSERT(color != Color());
            }
            {
                const auto mat = brightness(math::Vector3f(2.F, 1.F, 1.F));
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                const auto mat = contrast(math::Vector3f(2.F, 1.F, 1.F));
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                const auto mat = saturation(math::Vector3f(2.F, 1.F, 1.F));
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                const auto mat = tint(2.F);
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                Color color;
                color.brightness = math::Vector3f(2.F, 1.F, 1.F);
                color.contrast = math::Vector3f(2.F, 1.F, 1.F);
                color.saturation = math::Vector3f(2.F, 1.F, 1.F);
                color.tint = 2.F;
                color.invert = true;
                const auto mat = timeline::color(color);
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                Levels levels;
                levels.enabled = true;
                DTK_ASSERT(levels == levels);
                DTK_ASSERT(levels != Levels());
            }
            {
                EXRDisplay exrDisplay;
                exrDisplay.enabled = true;
                DTK_ASSERT(exrDisplay == exrDisplay);
                DTK_ASSERT(exrDisplay != EXRDisplay());
            }
            {
                SoftClip softClip;
                softClip.enabled = true;
                DTK_ASSERT(softClip == softClip);
                DTK_ASSERT(softClip != SoftClip());
            }
            {
                DisplayOptions displayOptions;
                displayOptions.channels = Channels::Red;
                DTK_ASSERT(displayOptions == displayOptions);
                DTK_ASSERT(displayOptions != DisplayOptions());
            }
        }
    }
}
