// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlQtTest/TimeObjectTest.h>

namespace tl
{
namespace qt_tests
    {
        TimeObjectTest::TimeObjectTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "qt_tests::TimeObjectTest")
        {}

        std::shared_ptr<TimeObjectTest> TimeObjectTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<TimeObjectTest>(new TimeObjectTest(context));
        }

        void TimeObjectTest::run()
        {}
    }
}
