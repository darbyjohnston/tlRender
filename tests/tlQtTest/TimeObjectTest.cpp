// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQtTest/TimeObjectTest.h>

namespace tl
{
    namespace tests
    {
        namespace qt_test
        {
            TimeObjectTest::TimeObjectTest(const std::shared_ptr<core::Context>& context) :
                ITest("qt_test::TimeObjectTest", context)
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
}
