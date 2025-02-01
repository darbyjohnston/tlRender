// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/LUTOptionsTest.h>

#include <tlTimeline/LUTOptions.h>

#include <dtk/core/Assert.h>
#include <dtk/core/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        LUTOptionsTest::LUTOptionsTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::LUTOptionsTest", context)
        {}

        std::shared_ptr<LUTOptionsTest> LUTOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<LUTOptionsTest>(new LUTOptionsTest(context));
        }

        void LUTOptionsTest::run()
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
        }
    }
}
