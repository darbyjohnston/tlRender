// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <opentimelineio/version.h>

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <chrono>
#include <iostream>

namespace tlr
{
    namespace otime = opentime::OPENTIME_VERSION;
    namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;

    //! Invalid time.
    const otime::RationalTime invalidTime(-1.0, -1.0);

    //! Invalid time range.
    const otime::TimeRange invalidTimeRange(invalidTime, invalidTime);

    //! Time.
    namespace time
    {
        //! Sleep for the given time.
        void sleep(const std::chrono::microseconds&);
    }

    std::ostream& operator << (std::ostream&, const otime::RationalTime&);
    std::ostream& operator << (std::ostream&, const otime::TimeRange&);

    std::istream& operator >> (std::istream&, otime::RationalTime&);
    std::istream& operator >> (std::istream&, otime::TimeRange&);
}
