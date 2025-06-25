// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCoreTest/TimeTest.h>

#include <tlCore/Time.h>

#include <feather-tk/core/Format.h>

#include <sstream>

using namespace tl::time;

namespace tl
{
    namespace core_tests
    {
        TimeTest::TimeTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "core_tests::TimeTest")
        {}

        std::shared_ptr<TimeTest> TimeTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<TimeTest>(new TimeTest(context));
        }

        void TimeTest::run()
        {
            _otime();
            _util();
            _keycode();
            _timecode();
            _serialize();
        }
        
        void TimeTest::_otime()
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
                FEATHER_TK_ASSERT(!isValid(invalidTime));
                FEATHER_TK_ASSERT(isValid(OTIO_NS::RationalTime(24.0, 24.0)));
            }
            {
                FEATHER_TK_ASSERT(!isValid(invalidTimeRange));
                FEATHER_TK_ASSERT(isValid(OTIO_NS::TimeRange(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::RationalTime(24.0, 24.0))));
            }
            {
                const OTIO_NS::TimeRange a(
                    OTIO_NS::RationalTime(24.0, 24.0),
                    OTIO_NS::RationalTime(24.0, 24.0));
                FEATHER_TK_ASSERT(compareExact(a, a));
                const OTIO_NS::TimeRange b(
                    OTIO_NS::RationalTime(1.0, 1.0),
                    OTIO_NS::RationalTime(1.0, 1.0));
                FEATHER_TK_ASSERT(a == b);
                FEATHER_TK_ASSERT(!compareExact(a, b));
            }
        }
        
        void TimeTest::_util()
        {
            {
                struct Data
                {
                    OTIO_NS::TimeRange range;
                    std::vector<OTIO_NS::RationalTime> frames;
                };
                const std::vector<Data> data =
                {
                    Data({ time::invalidTimeRange, {} }),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, 24.0),
                            OTIO_NS::RationalTime(1.0, 24.0)),
                        {
                            OTIO_NS::RationalTime(0.0, 24.0)
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, 24.0),
                            OTIO_NS::RationalTime(3.0, 24.0)),
                        {
                            OTIO_NS::RationalTime(0.0, 24.0),
                            OTIO_NS::RationalTime(1.0, 24.0),
                            OTIO_NS::RationalTime(2.0, 24.0)
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, 1.0),
                            OTIO_NS::RationalTime(1.0, 1.0)),
                        {
                            OTIO_NS::RationalTime(0.0, 1.0)
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, 1.0),
                            OTIO_NS::RationalTime(3.0, 1.0)),
                        {
                            OTIO_NS::RationalTime(0.0, 1.0),
                            OTIO_NS::RationalTime(1.0, 1.0),
                            OTIO_NS::RationalTime(2.0, 1.0)
                        }})
                };
                for (const auto& i : data)
                {
                    const auto frames = time::frames(i.range);
                    FEATHER_TK_ASSERT(frames == i.frames);
                }
            }
            {
                struct Data
                {
                    OTIO_NS::TimeRange range;
                    std::vector<OTIO_NS::TimeRange> seconds;
                };
                const std::vector<Data> data =
                {
                    Data({time::invalidTimeRange, {}}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, 24.0),
                            OTIO_NS::RationalTime(24.0, 24.0)),
                        {
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(0.0, 24.0),
                                OTIO_NS::RationalTime(24.0, 24.0))
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, 24.0),
                            OTIO_NS::RationalTime(72.0, 24.0)),
                        {
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(0.0, 24.0),
                                OTIO_NS::RationalTime(24.0, 24.0)),
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(24.0, 24.0),
                                OTIO_NS::RationalTime(24.0, 24.0)),
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(48.0, 24.0),
                                OTIO_NS::RationalTime(24.0, 24.0))
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(12.0, 24.0),
                            OTIO_NS::RationalTime(12.0, 24.0)),
                        {
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(12.0, 24.0),
                                OTIO_NS::RationalTime(12.0, 24.0))
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(12.0, 24.0),
                            OTIO_NS::RationalTime(24.0, 24.0)),
                        {
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(12.0, 24.0),
                                OTIO_NS::RationalTime(12.0, 24.0)),
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(24.0, 24.0),
                                OTIO_NS::RationalTime(12.0, 24.0))
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(23.0, 24.0),
                            OTIO_NS::RationalTime(24.0, 24.0)),
                        {
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(23.0, 24.0),
                                OTIO_NS::RationalTime(1.0, 24.0)),
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(24.0, 24.0),
                                OTIO_NS::RationalTime(23.0, 24.0))
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(-1.0, 24.0),
                            OTIO_NS::RationalTime(24.0, 24.0)),
                        {
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(-1.0, 24.0),
                                OTIO_NS::RationalTime(1.0, 24.0)),
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(0.0, 24.0),
                                OTIO_NS::RationalTime(23.0, 24.0))
                        }}),
                    Data({
                        OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(-1.0, 24.0),
                            OTIO_NS::RationalTime(48.0, 24.0)),
                        {
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(-1.0, 24.0),
                                OTIO_NS::RationalTime(1.0, 24.0)),
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(0.0, 24.0),
                                OTIO_NS::RationalTime(24.0, 24.0)),
                            OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(24.0, 24.0),
                                OTIO_NS::RationalTime(23.0, 24.0))
                        }})
                };
                for (const auto& i : data)
                {
                    const auto seconds = time::seconds(i.range);
                    FEATHER_TK_ASSERT(seconds == i.seconds);
                }
            }
            {
                struct Data
                {
                    double rate = 0.0;
                    std::pair<int, int> rational;
                };
                const std::vector<Data> data =
                {
                    Data({ 0.0, std::make_pair(0, 1)}),
                    Data({ 24.0, std::make_pair(24, 1)}),
                    Data({ 30.0, std::make_pair(30, 1)}),
                    Data({ 60.0, std::make_pair(60, 1)}),
                    Data({ 23.97602397602398, std::make_pair(24000, 1001)}),
                    Data({ 29.97002997002997, std::make_pair(30000, 1001)}),
                    Data({ 59.94005994005994, std::make_pair(60000, 1001)}),
                    Data({ 23.98, std::make_pair(24000, 1001)}),
                    Data({ 29.97, std::make_pair(30000, 1001)}),
                    Data({ 59.94, std::make_pair(60000, 1001)})
                };
                for (const auto& i : data)
                {
                    const auto rational = toRational(i.rate);
                    FEATHER_TK_ASSERT(
                        rational.first == i.rational.first &&
                        rational.second == i.rational.second);
                }
            }
        }
        
        void TimeTest::_keycode()
        {
            {
                const std::string s = keycodeToString(1, 2, 3, 4, 5);
                int id = 0;
                int type = 0;
                int prefix = 0;
                int count = 0;
                int offset = 0;
                stringToKeycode(s, id, type, prefix, count, offset);
                FEATHER_TK_ASSERT(1 == id);
                FEATHER_TK_ASSERT(2 == type);
                FEATHER_TK_ASSERT(3 == prefix);
                FEATHER_TK_ASSERT(4 == count);
                FEATHER_TK_ASSERT(5 == offset);
            }
            try
            {
                const std::string s = "...";
                int id = 0;
                int type = 0;
                int prefix = 0;
                int count = 0;
                int offset = 0;
                stringToKeycode(s, id, type, prefix, count, offset);
                FEATHER_TK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }
        
        void TimeTest::_timecode()
        {
            {
                const uint32_t t = timeToTimecode(1, 2, 3, 4);
                int hour = 0;
                int minute = 0;
                int second = 0;
                int frame = 0;
                timecodeToTime(t, hour, minute, second, frame);
                FEATHER_TK_ASSERT(1 == hour);
                FEATHER_TK_ASSERT(2 == minute);
                FEATHER_TK_ASSERT(3 == second);
                FEATHER_TK_ASSERT(4 == frame);
            }
            {
                const std::string s = "01:02:03:04";
                uint32_t t = 0;
                stringToTimecode(s, t);
                FEATHER_TK_ASSERT(s == timecodeToString(t));
            }
            try
            {
                const std::string s = "...";
                uint32_t t = 0;
                stringToTimecode(s, t);
                FEATHER_TK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
        }

        void TimeTest::_serialize()
        {
            {
                const OTIO_NS::RationalTime t(1.0, 24.0);
                nlohmann::json json;
                to_json(json, t);
                OTIO_NS::RationalTime t2 = invalidTime;
                from_json(json, t2);
                FEATHER_TK_ASSERT(t == t2);
            }
            {
                const auto t = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), OTIO_NS::RationalTime(1.0, 24.0));
                nlohmann::json json;
                to_json(json, t);
                OTIO_NS::TimeRange t2 = invalidTimeRange;
                from_json(json, t2);
                FEATHER_TK_ASSERT(t == t2);
            }
            {
                const auto t = OTIO_NS::RationalTime(1.0, 24.0);
                std::stringstream ss;
                ss << t;
                OTIO_NS::RationalTime t2 = invalidTime;
                ss >> t2;
                FEATHER_TK_ASSERT(t == t2);
            }
            try
            {
                OTIO_NS::RationalTime t = invalidTime;
                std::stringstream ss("...");
                ss >> t;
                FEATHER_TK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            {
                const auto t = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), OTIO_NS::RationalTime(1.0, 24.0));
                std::stringstream ss;
                ss << t;
                OTIO_NS::TimeRange t2 = invalidTimeRange;
                ss >> t2;
                FEATHER_TK_ASSERT(t == t2);
            }
            try
            {
                OTIO_NS::TimeRange t = invalidTimeRange;
                std::stringstream ss("...");
                ss >> t;
                FEATHER_TK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
            try
            {
                OTIO_NS::TimeRange t = invalidTimeRange;
                std::stringstream ss(".-.");
                ss >> t;
                FEATHER_TK_ASSERT(false);
            }
            catch (const std::exception&)
            {}
       }
    }
}
