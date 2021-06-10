// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ListObserverTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/ListObserver.h>

namespace tlr
{
    namespace CoreTest
    {
        ListObserverTest::ListObserverTest() :
            ITest("CoreTest::ListObserverTest")
        {}

        std::shared_ptr<ListObserverTest> ListObserverTest::create()
        {
            return std::shared_ptr<ListObserverTest>(new ListObserverTest);
        }

        void ListObserverTest::run()
        {
            std::vector<int> list = { 0 };
            auto value = observer::List<int>::create(list);
            TLR_ASSERT(list == value->get());

            std::vector<int> result;
            auto observer = observer::ListObserver<int>::create(
                value,
                [&result](const std::vector<int>& value)
                {
                    result = value;
                });
            list.push_back(1);
            bool changed = value->setIfChanged(list);
            TLR_ASSERT(changed);
            TLR_ASSERT(list == result);

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
                TLR_ASSERT(list == result);
                TLR_ASSERT(list == result2);

                TLR_ASSERT(2 == value->getObserversCount());
            }

            TLR_ASSERT(1 == value->getObserversCount());
        }
    }
}
