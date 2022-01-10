// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ValueObserverTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/ValueObserver.h>

namespace tlr
{
    namespace CoreTest
    {
        ValueObserverTest::ValueObserverTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::ValueObserverTest", context)
        {}

        std::shared_ptr<ValueObserverTest> ValueObserverTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<ValueObserverTest>(new ValueObserverTest(context));
        }

        void ValueObserverTest::run()
        {
            auto value = observer::Value<int>::create(0);
            TLR_ASSERT(0 == value->get());

            int result = 0;
            auto observer = observer::ValueObserver<int>::create(
                value,
                [&result](int value)
                {
                    result = value;
                });
            bool changed = value->setIfChanged(1);
            TLR_ASSERT(changed);
            TLR_ASSERT(1 == result);

            {
                int result2 = 0;
                auto observer2 = observer::ValueObserver<int>::create(
                    value,
                    [&result2](int value)
                    {
                        result2 = value;
                    });
                value->setIfChanged(2);
                TLR_ASSERT(2 == result);
                TLR_ASSERT(2 == result2);

                TLR_ASSERT(2 == value->getObserversCount());
            }

            TLR_ASSERT(1 == value->getObserversCount());
        }
    }
}
