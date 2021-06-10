// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/Time.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <sstream>

#if defined(_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

namespace tlr
{
    namespace time
    {
        void sleep(const std::chrono::microseconds& value)
        {
#if defined(_WINDOWS)
            if (HANDLE h = CreateWaitableTimer(NULL, TRUE, NULL))
            {
                LARGE_INTEGER l;
                l.QuadPart = -std::chrono::duration_cast<std::chrono::nanoseconds>(value).count() / 100;
                if (SetWaitableTimer(h, &l, 0, NULL, NULL, FALSE))
                {
                    WaitForSingleObject(h, INFINITE);
                }
                CloseHandle(h);
            }
#else
            const auto microseconds = value.count();
            const auto seconds = microseconds / 1000000;
            struct timespec t;
            t.tv_sec = seconds;
            t.tv_nsec = (microseconds - (seconds * 1000000)) * 1000;
            struct timespec out;
            nanosleep(&t, &out);
#endif
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
