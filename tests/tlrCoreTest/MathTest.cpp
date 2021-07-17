// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/MathTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Math.h>

using namespace tlr::math;

namespace tlr
{
    namespace CoreTest
    {
        MathTest::MathTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::MathTest", context)
        {}

        std::shared_ptr<MathTest> MathTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<MathTest>(new MathTest(context));
        }

        void MathTest::run()
        {
            {
                TLR_ASSERT(0 == clamp(-1, 0, 1));
                TLR_ASSERT(1 == clamp(2, 0, 1));
            }
        }
    }
}
