// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/LRUCacheTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/LRUCache.h>

using namespace tlr::memory;

namespace tlr
{
    namespace CoreTest
    {
        LRUCacheTest::LRUCacheTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::LRUCacheTest", context)
        {}

        std::shared_ptr<LRUCacheTest> LRUCacheTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<LRUCacheTest>(new LRUCacheTest(context));
        }

        void LRUCacheTest::run()
        {
            {
                LRUCache<int, int> c;
                TLR_ASSERT(0 == c.getSize());
                TLR_ASSERT(0.F == c.getPercentageUsed());
            }
            {
                LRUCache<int, int> c;
                TLR_ASSERT(!c.contains(0));
                int v = 0;
                TLR_ASSERT(!c.get(0, v));
                c.add(0, 1);
                TLR_ASSERT(1 == c.getSize());
                TLR_ASSERT(c.contains(0));
                TLR_ASSERT(c.get(0, v));
                TLR_ASSERT(1 == v);
                c.remove(0);
                TLR_ASSERT(!c.contains(0));
                c.add(0, 1);
                c.clear();
                TLR_ASSERT(!c.contains(0));
            }
            {
                LRUCache<int, int> c;
                c.setMax(3);
                TLR_ASSERT(3 == c.getMax());
                c.add(0, 1);
                c.add(1, 2);
                c.add(2, 3);
                c.add(3, 4);
                TLR_ASSERT(c.contains(1));
                TLR_ASSERT(c.contains(2));
                TLR_ASSERT(c.contains(3));
                int v = 0;
                c.get(1, v);
                c.add(4, 5);
                TLR_ASSERT(c.contains(3));
                TLR_ASSERT(c.contains(1));
                TLR_ASSERT(c.contains(4));
                const auto l = c.getKeys();
                TLR_ASSERT(std::vector<int>({ 1, 3, 4 }) == c.getKeys());
                TLR_ASSERT(std::vector<int>({ 2, 4, 5 }) == c.getValues());
            }
        }
    }
}
