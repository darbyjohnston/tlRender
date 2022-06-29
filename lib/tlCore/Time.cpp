// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/Time.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <array>
#include <cmath>
#include <sstream>

namespace tl
{
    namespace time
    {
        otime::RationalTime round(const otime::RationalTime& value)
        {
            return otime::RationalTime(std::round(value.value()), value.rate());
        }

        otime::RationalTime floor(const otime::RationalTime& value)
        {
            return otime::RationalTime(std::floor(value.value()), value.rate());
        }

        otime::RationalTime ceil(const otime::RationalTime& value)
        {
            return otime::RationalTime(std::ceil(value.value()), value.rate());
        }

        std::pair<int, int> toRational(double value)
        {
            const std::array<std::pair<int, int>, 6> common =
            {
                std::make_pair(24, 1),
                std::make_pair(30, 1),
                std::make_pair(60, 1),
                std::make_pair(24000, 1001),
                std::make_pair(30000, 1001),
                std::make_pair(60000, 1001)
            };
            const double tolerance = 0.01;
            for (const auto& i : common)
            {
                const double diff = fabs(value - i.first / static_cast<double>(i.second));
                if (diff < tolerance)
                {
                    return i;
                }
            }
            return std::make_pair(static_cast<int>(value), 1);
        }

        std::string keycodeToString(
            int id,
            int type,
            int prefix,
            int count,
            int offset)
        {
            std::vector<std::string> list;
            list.push_back(std::to_string(id));
            list.push_back(std::to_string(type));
            list.push_back(std::to_string(prefix));
            list.push_back(std::to_string(count));
            list.push_back(std::to_string(offset));
            return string::join(list, ":");
        }

        void stringToKeycode(
            const std::string& string,
            int& id,
            int& type,
            int& prefix,
            int& count,
            int& offset)
        {
            const auto pieces = string::split(string, ':');
            if (pieces.size() != 5)
            {
                throw error::ParseError();
            }
            id = std::stoi(pieces[0]);
            type = std::stoi(pieces[1]);
            prefix = std::stoi(pieces[2]);
            count = std::stoi(pieces[3]);
            offset = std::stoi(pieces[4]);
        }

        void timecodeToTime(
            uint32_t in,
            int& hour,
            int& minute,
            int& seconds,
            int& frame)
        {
            hour = (in >> 28 & 0x0f) * 10 + (in >> 24 & 0x0f);
            minute = (in >> 20 & 0x0f) * 10 + (in >> 16 & 0x0f);
            seconds = (in >> 12 & 0x0f) * 10 + (in >> 8 & 0x0f);
            frame = (in >> 4 & 0x0f) * 10 + (in >> 0 & 0x0f);
        }

        uint32_t timeToTimecode(
            int hour,
            int minute,
            int seconds,
            int frame)
        {
            return
                (hour / 10 & 0x0f) << 28 | (hour % 10 & 0x0f) << 24 |
                (minute / 10 & 0x0f) << 20 | (minute % 10 & 0x0f) << 16 |
                (seconds / 10 & 0x0f) << 12 | (seconds % 10 & 0x0f) << 8 |
                (frame / 10 & 0x0f) << 4 | (frame % 10 & 0x0f) << 0;
        }

        std::string timecodeToString(uint32_t in)
        {
            int hour = 0;
            int minute = 0;
            int second = 0;
            int frame = 0;
            timecodeToTime(in, hour, minute, second, frame);

            /*std::stringstream ss;
            ss << std::setfill('0') << std::setw(2) << hour;
            ss << std::setw(0) << ":";
            ss << std::setfill('0') << std::setw(2) << minute;
            ss << std::setw(0) << ":";
            ss << std::setfill('0') << std::setw(2) << second;
            ss << std::setw(0) << ":";
            ss << std::setfill('0') << std::setw(2) << frame;
            return ss.str();*/

            std::string out = "00:00:00:00";
            out[0] = 48 + hour / 10;
            out[1] = 48 + hour % 10;
            out[3] = 48 + minute / 10;
            out[4] = 48 + minute % 10;
            out[6] = 48 + second / 10;
            out[7] = 48 + second % 10;
            out[9] = 48 + frame / 10;
            out[10] = 48 + frame % 10;
            return out;
        }

        void stringToTimecode(const std::string& in, uint32_t& out)
        {
            int hour = 0;
            int minute = 0;
            int second = 0;
            int frame = 0;
            const auto pieces = string::split(in, ':');
            size_t i = 0;
            if (pieces.size() != 4)
            {
                throw error::ParseError();
            }
            hour = std::stoi(pieces[i]); ++i;
            minute = std::stoi(pieces[i]); ++i;
            second = std::stoi(pieces[i]); ++i;
            frame = std::stoi(pieces[i]);
            out = timeToTimecode(hour, minute, second, frame);
        }
    }
}

namespace opentime
{
    namespace OPENTIME_VERSION
    {
        std::ostream& operator << (std::ostream& os, const RationalTime& value)
        {
            os << std::fixed << value.value() << "/" << value.rate();
            return os;
        }

        std::ostream& operator << (std::ostream& os, const TimeRange& value)
        {
            os << std::fixed << value.start_time().value() << "-" <<
                value.end_time_inclusive().value() << "/" <<
                value.duration().rate();
            return os;
        }

        std::istream& operator >> (std::istream& is, RationalTime& out)
        {
            std::string s;
            is >> s;
            auto split = tl::string::split(s, '/');
            if (split.size() != 2)
            {
                throw tl::error::ParseError();
            }
            out = RationalTime(std::stof(split[0]), std::stof(split[1]));
            return is;
        }

        std::istream& operator >> (std::istream& is, TimeRange& out)
        {
            std::string s;
            is >> s;
            auto split = tl::string::split(s, '-');
            if (split.size() != 2)
            {
                throw tl::error::ParseError();
            }
            double startTime = 0.0;
            {
                std::stringstream ss(split[0]);
                ss >> startTime;
            }
            split = tl::string::split(split[1], '/');
            if (split.size() != 2)
            {
                throw tl::error::ParseError();
            }
            double endTime = 0.0;
            {
                std::stringstream ss(split[0]);
                ss >> endTime;
            }
            double rate = 0.0;
            {
                std::stringstream ss(split[1]);
                ss >> rate;
            }
            out = TimeRange::range_from_start_end_time_inclusive(
                RationalTime(startTime, rate),
                RationalTime(endTime, rate));
            return is;
        }
    }
}
