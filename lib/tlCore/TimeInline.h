// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <cmath>

namespace tl
{
    namespace time
    {
        inline bool isValid(const otime::RationalTime& value)
        {
            return !value.is_invalid_time();
        }

        inline bool isValid(const otime::TimeRange& value)
        {
            return
                !value.start_time().is_invalid_time() &&
                !value.duration().is_invalid_time();
        }

        constexpr bool compareExact(const otime::RationalTime& a, const otime::RationalTime& b)
        {
            return
                a.value() == b.value() &&
                a.rate() == b.rate();
        }

        constexpr bool compareExact(const otime::TimeRange& a, const otime::TimeRange& b)
        {
            return
                compareExact(a.start_time(), b.start_time()) &&
                compareExact(a.duration(), b.duration());
        }

        inline otime::RationalTime round(const otime::RationalTime& value)
        {
            return otime::RationalTime(std::round(value.value()), value.rate());
        }

        inline otime::RationalTime floor(const otime::RationalTime& value)
        {
            return otime::RationalTime(std::floor(value.value()), value.rate());
        }

        inline otime::RationalTime ceil(const otime::RationalTime& value)
        {
            return otime::RationalTime(std::ceil(value.value()), value.rate());
        }
    }
}

