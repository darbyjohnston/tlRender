// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQtTest/TimeObjectTest.h>

namespace tlr
{
    namespace QtTest
    {
        TimeObjectTest::TimeObjectTest(const std::shared_ptr<core::Context>& context) :
            ITest("QtTest::TimeObjectTest", context)
        {}

        std::shared_ptr<TimeObjectTest> TimeObjectTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<TimeObjectTest>(new TimeObjectTest(context));
        }

        void TimeObjectTest::run()
        {
        }
    }
}
