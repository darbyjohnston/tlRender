// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ColorTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Color.h>

using namespace tl::imaging;

namespace tl
{
    namespace CoreTest
    {
        ColorTest::ColorTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::ColorTest", context)
        {}

        std::shared_ptr<ColorTest> ColorTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<ColorTest>(new ColorTest(context));
        }

        void ColorTest::run()
        {
            {
                const Color4f c;
                TLRENDER_ASSERT(0.F == c.r);
                TLRENDER_ASSERT(0.F == c.g);
                TLRENDER_ASSERT(0.F == c.b);
                TLRENDER_ASSERT(0.F == c.a);
            }
            {
                const Color4f c(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(1.F == c.r);
                TLRENDER_ASSERT(2.F == c.g);
                TLRENDER_ASSERT(3.F == c.b);
                TLRENDER_ASSERT(4.F == c.a);
            }
            {
                TLRENDER_ASSERT(0 == fToU8(0.0));
                TLRENDER_ASSERT(255 == fToU8(1.0));
            }
        }
    }
}
