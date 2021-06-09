// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ColorTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Color.h>

using namespace tlr::imaging;

namespace tlr
{
    namespace CoreTest
    {
        ColorTest::ColorTest() :
            ITest("CoreTest::ColorTest")
        {}

        std::shared_ptr<ColorTest> ColorTest::create()
        {
            return std::shared_ptr<ColorTest>(new ColorTest);
        }

        void ColorTest::run()
        {
            {
                const Color4f c;
                TLR_ASSERT(0.F == c.r);
                TLR_ASSERT(0.F == c.g);
                TLR_ASSERT(0.F == c.b);
                TLR_ASSERT(0.F == c.a);
            }
            {
                const Color4f c(1.F, 2.F, 3.F, 4.F);
                TLR_ASSERT(1.F == c.r);
                TLR_ASSERT(2.F == c.g);
                TLR_ASSERT(3.F == c.b);
                TLR_ASSERT(4.F == c.a);
            }
            {
                TLR_ASSERT(0 == fToU8(0.0));
                TLR_ASSERT(255 == fToU8(1.0));
            }
        }
    }
}
