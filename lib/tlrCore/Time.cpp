// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Time.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <sstream>

using namespace tlr::core;

namespace tlr
{
    namespace time
    {
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
                //! \todo How can we translate this?
                throw std::runtime_error("Cannot parse the value");
            }
            id     = std::stoi(pieces[0]);
            type   = std::stoi(pieces[1]);
            prefix = std::stoi(pieces[2]);
            count  = std::stoi(pieces[3]);
            offset = std::stoi(pieces[4]);
        }
    }

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
