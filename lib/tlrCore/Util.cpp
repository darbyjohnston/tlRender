// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Util.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <sstream>

namespace tlr
{
    std::ostream& operator << (std::ostream& os, const otime::RationalTime& value)
    {
        os << value.value() << "/" << value.rate();
        return os;
    }

    std::ostream& operator << (std::ostream& os, const otime::TimeRange& value)
    {
        os << value.start_time() << "-" << value.duration();
        return os;
    }

    std::istream& operator >> (std::istream& is, otime::RationalTime& out)
    {
        std::string s;
        is >> s;
        auto split = string::split(s, '/');
        if (split.size() != 2)
        {
            throw ParseError();
        }
        out = otime::RationalTime(std::stof(split[0]), std::stof(split[1]));
        return is;
    }

    std::istream& operator >> (std::istream& is, otime::TimeRange& out)
    {
        std::string s;
        is >> s;
        auto split = string::split(s, '-');
        if (split.size() != 2)
        {
            throw ParseError();
        }
        otime::RationalTime start;
        otime::RationalTime duration;
        {
            std::stringstream ss(split[0]);
            ss >> start;
        }
        {
            std::stringstream ss(split[1]);
            ss >> duration;
        }
        out = otime::TimeRange(start, duration);
        return is;
    }
}
