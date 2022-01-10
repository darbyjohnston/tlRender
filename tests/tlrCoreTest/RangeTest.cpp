// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/RangeTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Range.h>

using namespace tlr::math;

namespace tlr
{
    namespace CoreTest
    {
        RangeTest::RangeTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::RangeTest", context)
        {}

        std::shared_ptr<RangeTest> RangeTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<RangeTest>(new RangeTest(context));
        }

        void RangeTest::run()
        {
            {
                const auto r = IntRange();
                TLR_ASSERT(0 == r.getMin());
                TLR_ASSERT(0 == r.getMax());
            }
            {
                const auto r = IntRange(1);
                TLR_ASSERT(1 == r.getMin());
                TLR_ASSERT(1 == r.getMax());
            }
            {
                const auto r = IntRange(1, 10);
                TLR_ASSERT(1 == r.getMin());
                TLR_ASSERT(10 == r.getMax());
            }
            {
                auto r = IntRange(1, 10);
                r.zero();
                TLR_ASSERT(0 == r.getMin());
                TLR_ASSERT(0 == r.getMax());
            }
            {
                const auto r = IntRange(1, 10);
                TLR_ASSERT(r.contains(1));
                TLR_ASSERT(r.contains(10));
                TLR_ASSERT(!r.contains(0));
                TLR_ASSERT(!r.contains(11));
            }
            {
                const auto r = IntRange(1, 10);
                TLR_ASSERT(r.intersects(IntRange(0, 1)));
                TLR_ASSERT(r.intersects(IntRange(10, 11)));
                TLR_ASSERT(!r.intersects(IntRange(12, 20)));
            }
            {
                auto r = IntRange(1, 10);
                r.expand(20);
                TLR_ASSERT(IntRange(1, 20) == r);
            }
            {
                auto r = IntRange(1, 10);
                r.expand(IntRange(0, 20));
                TLR_ASSERT(IntRange(0, 20) == r);
            }
            {
                TLR_ASSERT(IntRange(1, 10) == IntRange(1, 10));
                TLR_ASSERT(IntRange(1, 10) != IntRange(0, 11));
                TLR_ASSERT(IntRange(0, 10) < IntRange(1, 11));
            }
        }
    }
}
