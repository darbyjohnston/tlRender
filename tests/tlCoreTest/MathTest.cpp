// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/MathTest.h>

#include <tlCore/Math.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        MathTest::MathTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::MathTest", context)
        {}

        std::shared_ptr<MathTest> MathTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MathTest>(new MathTest(context));
        }

        void MathTest::run()
        {
            {
                DTK_ASSERT(rad2deg(deg2rad(360.F)) == 360.F);
            }
            {
                DTK_ASSERT(0 == clamp(-1, 0, 1));
                DTK_ASSERT(1 == clamp(2, 0, 1));
            }
            {
                DTK_ASSERT(0.F == lerp(0.F, 0.F, 1.F));
                DTK_ASSERT(1.F == lerp(1.F, 0.F, 1.F));
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
            {
                DTK_ASSERT(1 == digits(0));
                DTK_ASSERT(1 == digits(1));
                DTK_ASSERT(2 == digits(10));
                DTK_ASSERT(3 == digits(123));
                DTK_ASSERT(2 == digits(-1));
                DTK_ASSERT(3 == digits(-10));
                DTK_ASSERT(4 == digits(-123));
            }
        }
    }
}
