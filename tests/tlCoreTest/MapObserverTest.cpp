// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/MapObserverTest.h>

#include <tlCore/MapObserver.h>

namespace tl
{
    namespace core_tests
    {
        MapObserverTest::MapObserverTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::MapObserverTest", context)
        {}

        std::shared_ptr<MapObserverTest> MapObserverTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MapObserverTest>(new MapObserverTest(context));
        }

        void MapObserverTest::run()
        {
            std::map<int, int> map = {};
            auto value = observer::Map<int, int>::create(map);
            DTK_ASSERT(map == value->get());

            std::map<int, int> result;
            auto observer = observer::MapObserver<int, int>::create(
                value,
                [&result](const std::map<int, int>& value)
                {
                    result = value;
                });
            map[0] = 1;
            bool changed = value->setIfChanged(map);
            DTK_ASSERT(changed);
            changed = value->setIfChanged(map);
            DTK_ASSERT(!changed);
            DTK_ASSERT(map == result);
            DTK_ASSERT(1 == value->getSize());
            DTK_ASSERT(!value->isEmpty());
            DTK_ASSERT(value->hasKey(0));
            DTK_ASSERT(1 == value->getItem(0));

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
                DTK_ASSERT(map == result);
                DTK_ASSERT(map == result2);
                DTK_ASSERT(2 == value->getSize());
                DTK_ASSERT(!value->isEmpty());
                DTK_ASSERT(value->hasKey(1));
                DTK_ASSERT(2 == value->getItem(1));
                DTK_ASSERT(2 == value->getObserversCount());
            }

            DTK_ASSERT(1 == value->getObserversCount());
        }
    }
}
