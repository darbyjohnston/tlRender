// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ListObserverTest.h>

#include <tlCore/ListObserver.h>

namespace tl
{
    namespace core_tests
    {
        ListObserverTest::ListObserverTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ListObserverTest", context)
        {}

        std::shared_ptr<ListObserverTest> ListObserverTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ListObserverTest>(new ListObserverTest(context));
        }

        void ListObserverTest::run()
        {
            std::vector<int> list = {};
            auto value = observer::List<int>::create(list);
            DTK_ASSERT(list == value->get());

            std::vector<int> result;
            auto observer = observer::ListObserver<int>::create(
                value,
                [&result](const std::vector<int>& value)
                {
                    result = value;
                });
            list.push_back(1);
            bool changed = value->setIfChanged(list);
            DTK_ASSERT(changed);
            changed = value->setIfChanged(list);
            DTK_ASSERT(!changed);
            DTK_ASSERT(list == result);
            DTK_ASSERT(1 == value->getSize());
            DTK_ASSERT(!value->isEmpty());
            DTK_ASSERT(1 == value->getItem(0));
            DTK_ASSERT(value->contains(1));
            DTK_ASSERT(0 == value->indexOf(1));

            {
                std::vector<int> result2;
                auto observer2 = observer::ListObserver<int>::create(
                    value,
                    [&result2](const std::vector<int>& value)
                    {
                        result2 = value;
                    });
                list.push_back(2);
                value->setIfChanged(list);
                DTK_ASSERT(list == result);
                DTK_ASSERT(list == result2);
                DTK_ASSERT(2 == value->getSize());
                DTK_ASSERT(2 == value->getItem(1));
                DTK_ASSERT(value->contains(2));
                DTK_ASSERT(1 == value->indexOf(2));
                DTK_ASSERT(2 == value->getObserversCount());
            }

            DTK_ASSERT(1 == value->getObserversCount());

            value->clear();
            DTK_ASSERT(value->isEmpty());
            value->pushBack(2);
            value->pushBack(3);
            value->setItem(0, 4);
            value->setItemOnlyIfChanged(1, 5);
            value->setItemOnlyIfChanged(1, 5);
            value->pushBack(6);
            value->removeItem(0);
            value->removeItem(0);
            value->removeItem(0);
            DTK_ASSERT(value->isEmpty());
        }
    }
}
