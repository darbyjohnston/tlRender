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
        MapObserverTest::MapObserverTest() :
            ITest("CoreTest::MapObserverTest")
        {}

        std::shared_ptr<MapObserverTest> MapObserverTest::create()
        {
            return std::shared_ptr<MapObserverTest>(new MapObserverTest);
        }

        void MapObserverTest::run()
        {
            std::map<int, int> map = { { 0, 1 } };
            auto value = observer::Map<int, int>::create(map);
            TLR_ASSERT(map == value->get());

            std::map<int, int> result;
            auto observer = observer::MapObserver<int, int>::create(
                value,
                [&result](const std::map<int, int>& value)
                {
                    result = value;
                });
            map[1] = 2;
            bool changed = value->setIfChanged(map);
            TLR_ASSERT(changed);
            TLR_ASSERT(map == result);

            {
                std::map<int, int> result2;
                auto observer2 = observer::MapObserver<int, int>::create(
                    value,
                    [&result2](const std::map<int, int>& value)
                    {
                        result2 = value;
                    });
                map[2] = 3;
                value->setIfChanged(map);
                TLR_ASSERT(map == result);
                TLR_ASSERT(map == result2);

                TLR_ASSERT(2 == value->getObserversCount());
            }

            TLR_ASSERT(1 == value->getObserversCount());
        }
    }
}
