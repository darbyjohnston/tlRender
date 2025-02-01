// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/OCIOOptionsTest.h>

#include <tlTimeline/OCIOOptions.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        OCIOOptionsTest::OCIOOptionsTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::OCIOOptionsTest", context)
        {}

        std::shared_ptr<OCIOOptionsTest> OCIOOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<OCIOOptionsTest>(new OCIOOptionsTest(context));
        }

        void OCIOOptionsTest::run()
        {
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
