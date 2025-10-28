// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/ColorOptionsTest.h>

#include <tlTimeline/ColorOptions.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        ColorOptionsTest::ColorOptionsTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "timeline_tests::ColorOptionsTest")
        {}

        std::shared_ptr<ColorOptionsTest> ColorOptionsTest::create(const std::shared_ptr<ftk::Context>& context)
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
                _print("LUT format names: " + ftk::join(getLUTFormatNames(), ", "));
            }
            {
                _print("LUT format extensions: " + ftk::join(getLUTFormatExtensions(), ", "));
            }
            {
                OCIOOptions a;
                OCIOOptions b;
                FTK_ASSERT(a == b);
                a.fileName = "fileName";
                FTK_ASSERT(a != b);
            }
        }
    }
}
