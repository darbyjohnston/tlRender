// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <opentimelineio/version.h>

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <iostream>

namespace tl
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

        //! Check whether the given time is valid. This function should be
        //! used instead of comparing a time to the "invalidTime" constant.
        bool isValid(const otime::RationalTime&);

        //! Check whether the given time range is valid. This function
        //! should be used instead of comparing a time range to the
        //! "invalidTimeRange" constant.
        bool isValid(const otime::TimeRange&);

        //! Round the given time.
        otime::RationalTime round(const otime::RationalTime&);

        //! Round the given time downward.
        otime::RationalTime floor(const otime::RationalTime&);

        //! Round the given time upward.
        otime::RationalTime ceil(const otime::RationalTime&);

        //! Get the frames in a time range.
        std::vector<otime::RationalTime> frames(const otime::TimeRange&);

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
        void to_json(nlohmann::json&, const RationalTime&);
        void to_json(nlohmann::json&, const TimeRange&);

        void from_json(const nlohmann::json&, RationalTime&);
        void from_json(const nlohmann::json&, TimeRange&);

        std::ostream& operator << (std::ostream&, const RationalTime&);
        std::ostream& operator << (std::ostream&, const TimeRange&);

        std::istream& operator >> (std::istream&, RationalTime&);
        std::istream& operator >> (std::istream&, TimeRange&);
    }
}
