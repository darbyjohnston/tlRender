// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ValueObserverTest.h>

#include <tlCore/Assert.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace core_tests
    {
        ValueObserverTest::ValueObserverTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ValueObserverTest", context)
        {}

        std::shared_ptr<ValueObserverTest> ValueObserverTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ValueObserverTest>(new ValueObserverTest(context));
        }

        void ValueObserverTest::run()
        {
            auto value = observer::Value<int>::create(0);
            TLRENDER_ASSERT(0 == value->get());

            int result = 0;
            auto observer = observer::ValueObserver<int>::create(
                value,
                [&result](int value)
                {
                    result = value;
                });
            bool changed = value->setIfChanged(1);
            TLRENDER_ASSERT(changed);
            TLRENDER_ASSERT(1 == result);

            {
                int result2 = 0;
                auto observer2 = observer::ValueObserver<int>::create(
                    value,
                    [&result2](int value)
                    {
                        result2 = value;
                    });
                value->setIfChanged(2);
                TLRENDER_ASSERT(2 == result);
                TLRENDER_ASSERT(2 == result2);

                TLRENDER_ASSERT(2 == value->getObserversCount());
            }

            TLRENDER_ASSERT(1 == value->getObserversCount());
        }
    }
}
