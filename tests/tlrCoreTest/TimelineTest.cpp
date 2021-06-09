// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/TimelineTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Timeline.h>

namespace tlr
{
    namespace CoreTest
    {
        TimelineTest::TimelineTest() :
            ITest("CoreTest::TimelineTest")
        {}

        std::shared_ptr<TimelineTest> TimelineTest::create()
        {
            return std::shared_ptr<TimelineTest>(new TimelineTest);
        }

        void TimelineTest::run()
        {
            _toRanges();
        }

        void TimelineTest::_toRanges()
        {
            {
                std::vector<otime::RationalTime> f;
                auto r = timeline::toRanges(f);
                TLR_ASSERT(r.empty());
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24)
                };
                auto r = timeline::toRanges(f);
                TLR_ASSERT(1 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(1, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24)
                };
                auto r = timeline::toRanges(f);
                TLR_ASSERT(1 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(2, 24)
                };
                auto r = timeline::toRanges(f);
                TLR_ASSERT(1 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(3, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(2, 24)
                };
                auto r = timeline::toRanges(f);
                TLR_ASSERT(2 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(1, 24)) == r[0]);
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(2, 24), otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24)
                };
                auto r = timeline::toRanges(f);
                TLR_ASSERT(2 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(3, 24), otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f =
                {
                    otime::RationalTime(0, 24),
                    otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24),
                    otime::RationalTime(4, 24)
                };
                auto r = timeline::toRanges(f);
                TLR_ASSERT(2 == r.size());
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(0, 24), otime::RationalTime(2, 24)) == r[0]);
                TLR_ASSERT(otime::TimeRange(otime::RationalTime(3, 24), otime::RationalTime(2, 24)) == r[1]);
            }
        }
    }
}
