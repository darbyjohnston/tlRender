// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TimeTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Time.h>

#include <array>
#include <sstream>

using namespace tlr::time;

namespace tlr
{
    namespace CoreTest
    {
        TimeTest::TimeTest(const std::shared_ptr<core::Context>& context) :
            ITest("CoreTest::TimeTest", context)
        {}

        std::shared_ptr<TimeTest> TimeTest::create(const std::shared_ptr<core::Context>& context)
        {
            return std::shared_ptr<TimeTest>(new TimeTest(context));
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
                struct Data
                {
                    double rate = 0.0;
                    std::pair<int, int> rational;
                };
                const std::array<Data, 10> data =
                {
                    Data({ 0.0, std::make_pair( 0, 1 )}),
                    Data({ 24.0, std::make_pair(24, 1 )}),
                    Data({ 30.0, std::make_pair(30, 1 )}),
                    Data({ 60.0, std::make_pair(60, 1 )}),
                    Data({ 23.97602397602398, std::make_pair(24000, 1001 )}),
                    Data({ 29.97002997002997, std::make_pair(30000, 1001 )}),
                    Data({ 59.94005994005994, std::make_pair(60000, 1001 )}),
                    Data({ 23.98, std::make_pair(24000, 1001 )}),
                    Data({ 29.97, std::make_pair(30000, 1001 )}),
                    Data({ 59.94, std::make_pair(60000, 1001 )})
                };
                for (const auto& i : data)
                {
                    const auto rational = toRational(i.rate);
                    TLR_ASSERT(rational.first == i.rational.first && rational.second == i.rational.second);
                }
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
