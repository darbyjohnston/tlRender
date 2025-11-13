// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <opentimelineio/version.h>

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <iostream>

namespace tl
{
    //! Time
    namespace time
    {
        //! Invalid time.
        constexpr OTIO_NS::RationalTime invalidTime(-1.0, -1.0);

        //! Invalid time range.
        constexpr OTIO_NS::TimeRange invalidTimeRange(invalidTime, invalidTime);

        //! Check whether the given time is valid. This function should be
        //! used instead of comparing a time to the "invalidTime" constant.
        inline bool isValid(const OTIO_NS::RationalTime&);

        //! Check whether the given time range is valid. This function
        //! should be used instead of comparing a time range to the
        //! "invalidTimeRange" constant.
        inline bool isValid(const OTIO_NS::TimeRange&);

        //! Compare two time ranges. This function compares the values
        //! exactly, unlike the "==" operator which rescales the values.
        constexpr bool compareExact(const OTIO_NS::TimeRange&, const OTIO_NS::TimeRange&);

        //! Get the frames in a time range.
        std::vector<OTIO_NS::RationalTime> frames(const OTIO_NS::TimeRange&);

        //! Split a time range at into seconds.
        std::vector<OTIO_NS::TimeRange> seconds(const OTIO_NS::TimeRange&);

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
        std::string to_string(const RationalTime&);
        std::string to_string(const TimeRange&);

        bool from_string(const std::string&, RationalTime&);
        bool from_string(const std::string&, TimeRange&);

        void to_json(nlohmann::json&, const RationalTime&);
        void to_json(nlohmann::json&, const TimeRange&);

        void from_json(const nlohmann::json&, RationalTime&);
        void from_json(const nlohmann::json&, TimeRange&);

        std::ostream& operator << (std::ostream&, const RationalTime&);
        std::ostream& operator << (std::ostream&, const TimeRange&);

        bool cmdLineParse(std::vector<std::string>&, std::vector<std::string>::iterator&, RationalTime&);
        bool cmdLineParse(std::vector<std::string>&, std::vector<std::string>::iterator&, TimeRange&);
    }
}

#include <tlCore/TimeInline.h>

