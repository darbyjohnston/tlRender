// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/ColorOptionsTest.h>

#include <tlTimeline/ColorOptions.h>

#include <feather-tk/core/Assert.h>
#include <feather-tk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        ColorOptionsTest::ColorOptionsTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "timeline_tests::ColorOptionsTest")
        {}

        std::shared_ptr<ColorOptionsTest> ColorOptionsTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<ColorOptionsTest>(new ColorOptionsTest(context));
        }

        void ColorOptionsTest::run()
        {
            {
                _enum<OCIOConfig>("OCIOConfig", getOCIOConfigEnums);
            }
            {
                _enum<LUTOrder>("LUTOrder", getLUTOrderEnums);
            }
            {
                _print("LUT format names: " + feather_tk::join(getLUTFormatNames(), ", "));
            }
            {
                _print("LUT format extensions: " + feather_tk::join(getLUTFormatExtensions(), ", "));
            }
            {
                OCIOOptions a;
                OCIOOptions b;
                FEATHER_TK_ASSERT(a == b);
                a.fileName = "fileName";
                FEATHER_TK_ASSERT(a != b);
            }
        }
    }
}
