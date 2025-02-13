// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/ColorOptionsTest.h>

#include <tlTimeline/ColorOptions.h>

#include <dtk/core/Assert.h>
#include <dtk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        ColorOptionsTest::ColorOptionsTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "timeline_tests::ColorOptionsTest")
        {}

        std::shared_ptr<ColorOptionsTest> ColorOptionsTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<ColorOptionsTest>(new ColorOptionsTest(context));
        }

        void ColorOptionsTest::run()
        {
            {
                _enum<LUTOrder>("LUTOrder", getLUTOrderEnums);
            }
            {
                _print("LUT format names: " + dtk::join(getLUTFormatNames(), ", "));
            }
            {
                _print("LUT format extensions: " + dtk::join(getLUTFormatExtensions(), ", "));
            }
            {
                OCIOOptions a;
                OCIOOptions b;
                DTK_ASSERT(a == b);
                a.fileName = "fileName";
                DTK_ASSERT(a != b);
            }
        }
    }
}
