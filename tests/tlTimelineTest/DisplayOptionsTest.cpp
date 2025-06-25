// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/DisplayOptionsTest.h>

#include <tlTimeline/DisplayOptions.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        DisplayOptionsTest::DisplayOptionsTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "timeline_tests::DisplayOptionsTest")
        {}

        std::shared_ptr<DisplayOptionsTest> DisplayOptionsTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<DisplayOptionsTest>(new DisplayOptionsTest(context));
        }

        void DisplayOptionsTest::run()
        {
            {
                Color color;
                color.enabled = true;
                FEATHER_TK_ASSERT(color == color);
                FEATHER_TK_ASSERT(color != Color());
            }
            {
                Color color;
                color.brightness = feather_tk::V3F(2.F, 1.F, 1.F);
                color.contrast = feather_tk::V3F(2.F, 1.F, 1.F);
                color.saturation = feather_tk::V3F(2.F, 1.F, 1.F);
                color.tint = 2.F;
                color.invert = true;
                const feather_tk::V3F v(1.F, 1.F, 1.F);
                const auto m = timeline::color(color);
                _print(feather_tk::Format("{0} color: {1}").arg(v).arg(v * m));
            }
            {
                Levels levels;
                levels.enabled = true;
                FEATHER_TK_ASSERT(levels == levels);
                FEATHER_TK_ASSERT(levels != Levels());
            }
            {
                EXRDisplay exrDisplay;
                exrDisplay.enabled = true;
                FEATHER_TK_ASSERT(exrDisplay == exrDisplay);
                FEATHER_TK_ASSERT(exrDisplay != EXRDisplay());
            }
            {
                SoftClip softClip;
                softClip.enabled = true;
                FEATHER_TK_ASSERT(softClip == softClip);
                FEATHER_TK_ASSERT(softClip != SoftClip());
            }
            {
                DisplayOptions displayOptions;
                displayOptions.channels = feather_tk::ChannelDisplay::Red;
                FEATHER_TK_ASSERT(displayOptions == displayOptions);
                FEATHER_TK_ASSERT(displayOptions != DisplayOptions());
            }
        }
    }
}
