// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/MathTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Math.h>

using namespace tl::math;

namespace tl
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
                TLRENDER_ASSERT(0 == clamp(-1, 0, 1));
                TLRENDER_ASSERT(1 == clamp(2, 0, 1));
            }
            {
                TLRENDER_ASSERT(0.F == lerp(0.F, 0.F, 1.F));
                TLRENDER_ASSERT(1.F == lerp(1.F, 0.F, 1.F));
            }
            {
                for (float i = 0.F; i <= 1.F; i += .1F)
                {
                    std::stringstream ss;
                    ss << "Smoothstep " << i << ": " << smoothStep(i, 0.F, 1.F);
                    _print(ss.str());
                }
                for (double i = 0.0; i <= 1.0; i += 0.1)
                {
                    std::stringstream ss;
                    ss << "Smoothstep " << i << ": " << smoothStep(i, 0.0, 1.0);
                    _print(ss.str());
                }
            }
        }
    }
}
