// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQtTest/TimeObjectTest.h>

namespace tlr
{
    namespace QtTest
    {
        TimeObjectTest::TimeObjectTest() :
            ITest("QtTest::TimeObjectTest")
        {}

        std::shared_ptr<TimeObjectTest> TimeObjectTest::create()
        {
            return std::shared_ptr<TimeObjectTest>(new TimeObjectTest);
        }

        void TimeObjectTest::run()
        {
        }
    }
}
