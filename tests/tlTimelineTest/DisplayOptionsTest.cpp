// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/DisplayOptionsTest.h>

#include <tlTimeline/DisplayOptions.h>

#include <dtk/core/Assert.h>
#include <dtk/core/Format.h>
#include <dtk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        DisplayOptionsTest::DisplayOptionsTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "timeline_tests::DisplayOptionsTest")
        {}

        std::shared_ptr<DisplayOptionsTest> DisplayOptionsTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<DisplayOptionsTest>(new DisplayOptionsTest(context));
        }

        void DisplayOptionsTest::run()
        {
            {
                Color color;
                color.enabled = true;
                DTK_ASSERT(color == color);
                DTK_ASSERT(color != Color());
            }
            {
                Color color;
                color.brightness = dtk::V3F(2.F, 1.F, 1.F);
                color.contrast = dtk::V3F(2.F, 1.F, 1.F);
                color.saturation = dtk::V3F(2.F, 1.F, 1.F);
                color.tint = 2.F;
                color.invert = true;
                const dtk::V3F v(1.F, 1.F, 1.F);
                const auto m = timeline::color(color);
                _print(dtk::Format("{0} color: {1}").arg(v).arg(v * m));
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
                displayOptions.channels = dtk::ChannelDisplay::Red;
                DTK_ASSERT(displayOptions == displayOptions);
                DTK_ASSERT(displayOptions != DisplayOptions());
            }
        }
    }
}
