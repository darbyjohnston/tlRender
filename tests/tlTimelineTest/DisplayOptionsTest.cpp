// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/DisplayOptionsTest.h>

#include <tlTimeline/DisplayOptions.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        DisplayOptionsTest::DisplayOptionsTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::DisplayOptionsTest")
        {}

        std::shared_ptr<DisplayOptionsTest> DisplayOptionsTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<DisplayOptionsTest>(new DisplayOptionsTest(context));
        }

        void DisplayOptionsTest::run()
        {
            {
                Color color;
                color.enabled = true;
                FTK_ASSERT(color == color);
                FTK_ASSERT(color != Color());
            }
            {
                Color color;
                color.brightness = ftk::V3F(2.F, 1.F, 1.F);
                color.contrast = ftk::V3F(2.F, 1.F, 1.F);
                color.saturation = ftk::V3F(2.F, 1.F, 1.F);
                color.tint = 2.F;
                color.invert = true;
                const ftk::V3F v(1.F, 1.F, 1.F);
                const auto m = timeline::color(color);
                _print(ftk::Format("{0} color: {1}").arg(v).arg(m * v));
            }
            {
                Levels levels;
                levels.enabled = true;
                FTK_ASSERT(levels == levels);
                FTK_ASSERT(levels != Levels());
            }
            {
                EXRDisplay exrDisplay;
                exrDisplay.enabled = true;
                FTK_ASSERT(exrDisplay == exrDisplay);
                FTK_ASSERT(exrDisplay != EXRDisplay());
            }
            {
                SoftClip softClip;
                softClip.enabled = true;
                FTK_ASSERT(softClip == softClip);
                FTK_ASSERT(softClip != SoftClip());
            }
            {
                DisplayOptions displayOptions;
                displayOptions.channels = ftk::ChannelDisplay::Red;
                FTK_ASSERT(displayOptions == displayOptions);
                FTK_ASSERT(displayOptions != DisplayOptions());
            }
        }
    }
}
