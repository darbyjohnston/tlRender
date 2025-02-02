// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/LRUCacheTest.h>

#include <tlCore/LRUCache.h>
#include <tlCore/Memory.h>

using namespace tl::memory;

namespace tl
{
    namespace core_tests
    {
        LRUCacheTest::LRUCacheTest(const std::shared_ptr<dtk::Context>& context) :
            ITest(context, "core_tests::LRUCacheTest")
        {}

        std::shared_ptr<LRUCacheTest> LRUCacheTest::create(const std::shared_ptr<dtk::Context>& context)
        {
            return std::shared_ptr<LRUCacheTest>(new LRUCacheTest(context));
        }

        void LRUCacheTest::run()
        {
            {
                LRUCache<int, int> c;
                DTK_ASSERT(0 == c.getSize());
                DTK_ASSERT(0.F == c.getPercentage());
            }
            {
                LRUCache<int, int> c;
                DTK_ASSERT(!c.contains(0));
                int v = 0;
                DTK_ASSERT(!c.get(0, v));
                c.add(0, 1);
                DTK_ASSERT(1 == c.getSize());
                DTK_ASSERT(c.contains(0));
                DTK_ASSERT(c.get(0, v));
                DTK_ASSERT(1 == v);
                c.remove(0);
                DTK_ASSERT(!c.contains(0));
                c.add(0, 1);
                c.clear();
                DTK_ASSERT(!c.contains(0));
            }
            {
                LRUCache<int, int> c;
                c.setMax(3);
                DTK_ASSERT(3 == c.getMax());
                c.add(0, 1);
                c.add(1, 2);
                c.add(2, 3);
                c.add(3, 4);
                DTK_ASSERT(c.contains(1));
                DTK_ASSERT(c.contains(2));
                DTK_ASSERT(c.contains(3));
                int v = 0;
                c.get(1, v);
                c.add(4, 5);
                DTK_ASSERT(c.contains(1));
                DTK_ASSERT(c.contains(3));
                DTK_ASSERT(c.contains(4));
                const auto l = c.getKeys();
                DTK_ASSERT(std::vector<int>({ 1, 3, 4 }) == c.getKeys());
                DTK_ASSERT(std::vector<int>({ 2, 4, 5 }) == c.getValues());
            }
            {
                LRUCache<int, int> c;
                c.setMax(3 * memory::megabyte);
                c.add(0, 1, memory::megabyte);
                c.add(1, 2, memory::megabyte);
                c.add(2, 3, memory::megabyte);
                c.add(3, 4, memory::megabyte);
                DTK_ASSERT(c.contains(1));
                DTK_ASSERT(c.contains(2));
                DTK_ASSERT(c.contains(3));
                int v = 0;
                c.get(1, v);
                c.add(4, 5, memory::megabyte);
                DTK_ASSERT(c.contains(1));
                DTK_ASSERT(c.contains(3));
                DTK_ASSERT(c.contains(4));
                const auto l = c.getKeys();
                DTK_ASSERT(std::vector<int>({ 1, 3, 4 }) == c.getKeys());
                DTK_ASSERT(std::vector<int>({ 2, 4, 5 }) == c.getValues());
            }
        }
    }
}
