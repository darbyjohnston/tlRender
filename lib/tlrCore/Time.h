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

    //! Time.
    namespace time
    {
        //! Invalid time.
        const otime::RationalTime invalidTime(-1.0, -1.0);

        //! Invalid time range.
        const otime::TimeRange invalidTimeRange(invalidTime, invalidTime);

        //! Sleep for the given time.
        void sleep(const std::chrono::microseconds&);

        //! Convert a floating point rate to a rational.
        std::pair<int, int> toRational(double);

        //! \name Keycode
        ///@{

        std::string keycodeToString(
            int id,
            int type,
            int prefix,
            int count,
            int offset);

        void stringToKeycode(
            const std::string&,
            int& id,
            int& type,
            int& prefix,
            int& count,
            int& offset);

        ///@}

        //! \name Timecode
        ///@{

        void timecodeToTime(
            uint32_t,
            int& hour,
            int& minute,
            int& second,
            int& frame);

        uint32_t timeToTimecode(
            int hour,
            int minute,
            int second,
            int frame);

        std::string timecodeToString(uint32_t);

        void stringToTimecode(const std::string&, uint32_t&);

        ///@}
    }
}

namespace opentime
{
    namespace OPENTIME_VERSION
    {
        std::ostream& operator << (std::ostream&, const RationalTime&);
        std::ostream& operator << (std::ostream&, const TimeRange&);

        std::istream& operator >> (std::istream&, RationalTime&);
        std::istream& operator >> (std::istream&, TimeRange&);
    }
}
