// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timeline
    {
        TLRENDER_ENUM_IMPL(TimerMode, "System", "Audio");
        TLRENDER_ENUM_SERIALIZE_IMPL(TimerMode);

        TLRENDER_ENUM_IMPL(ExternalTimeMode, "Relative", "Absolute");
        TLRENDER_ENUM_SERIALIZE_IMPL(ExternalTimeMode);

        otime::RationalTime getExternalTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& externalTimeRange,
            ExternalTimeMode mode)
        {
            otime::RationalTime out;
            switch (mode)
            {
            case ExternalTimeMode::Relative:
            {
                const otime::RationalTime relativeTime =
                    sourceTime - sourceTimeRange.start_time();
                const otime::RationalTime relativeTimeRescaled = time::floor(
                    relativeTime.rescaled_to(externalTimeRange.duration().rate()));
                out = externalTimeRange.start_time() + relativeTimeRescaled;
                break;
            }
            case ExternalTimeMode::Absolute:
                out = time::floor(sourceTime.rescaled_to(
                    externalTimeRange.duration().rate()));
                break;
            default: break;
            }
            return out;
        }
    }
}
