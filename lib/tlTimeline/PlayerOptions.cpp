// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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

        TLRENDER_ENUM_IMPL(AudioBufferFrameCount, "16", "32", "64", "128", "256", "512", "1024");
        TLRENDER_ENUM_SERIALIZE_IMPL(AudioBufferFrameCount);

        size_t getAudioBufferFrameCount(AudioBufferFrameCount value)
        {
            const std::array<size_t, static_cast<size_t>(AudioBufferFrameCount::Count)> data =
            {
                16,
                32,
                64,
                128,
                256,
                512,
                1024
            };
            return data[static_cast<size_t>(value)];
        }

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
