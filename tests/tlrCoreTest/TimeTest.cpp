// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TimeTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Time.h>

#include <sstream>

using namespace tlr::time;

namespace tlr
{
    namespace CoreTest
    {
        TimeTest::TimeTest() :
            ITest("CoreTest::TimeTest")
        {}

        std::shared_ptr<TimeTest> TimeTest::create()
        {
            return std::shared_ptr<TimeTest>(new TimeTest);
        }

        void TimeTest::run()
        {
            {
                std::stringstream ss;
                ss << "Invalid time: " << invalidTime;
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "Invalid time range: " << invalidTimeRange;
                _print(ss.str());
            }
            {
                sleep(std::chrono::microseconds(1000000));
            }
            {
                const auto t = otime::RationalTime(1.0, 24.0);
                std::stringstream ss;
                ss << t;
                otime::RationalTime t2 = invalidTime;
                ss >> t2;
                TLR_ASSERT(t == t2);
            }
            {
                const auto t = otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                std::stringstream ss;
                ss << t;
                otime::TimeRange t2 = invalidTimeRange;
                ss >> t2;
                TLR_ASSERT(t == t2);
            }
        }
    }
}
