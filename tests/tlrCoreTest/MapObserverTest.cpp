// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/MapObserverTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/MapObserver.h>

namespace tlr
{
    namespace CoreTest
    {
        MapObserverTest::MapObserverTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::MapObserverTest", context)
        {}

        std::shared_ptr<MapObserverTest> MapObserverTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<MapObserverTest>(new MapObserverTest(context));
        }

        void MapObserverTest::run()
        {
            std::map<int, int> map = {};
            auto value = observer::Map<int, int>::create(map);
            TLR_ASSERT(map == value->get());

            std::map<int, int> result;
            auto observer = observer::MapObserver<int, int>::create(
                value,
                [&result](const std::map<int, int>& value)
                {
                    result = value;
                });
            map[0] = 1;
            bool changed = value->setIfChanged(map);
            TLR_ASSERT(changed);
            changed = value->setIfChanged(map);
            TLR_ASSERT(!changed);
            TLR_ASSERT(map == result);
            TLR_ASSERT(1 == value->getSize());
            TLR_ASSERT(!value->isEmpty());
            TLR_ASSERT(value->hasKey(0));
            TLR_ASSERT(1 == value->getItem(0));

            {
                std::map<int, int> result2;
                auto observer2 = observer::MapObserver<int, int>::create(
                    value,
                    [&result2](const std::map<int, int>& value)
                    {
                        result2 = value;
                    });
                map[1] = 2;
                value->setIfChanged(map);
                TLR_ASSERT(map == result);
                TLR_ASSERT(map == result2);
                TLR_ASSERT(2 == value->getSize());
                TLR_ASSERT(!value->isEmpty());
                TLR_ASSERT(value->hasKey(1));
                TLR_ASSERT(2 == value->getItem(1));
                TLR_ASSERT(2 == value->getObserversCount());
            }

            TLR_ASSERT(1 == value->getObserversCount());
        }
    }
}
