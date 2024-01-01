// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Time.h>

namespace tl
{
    namespace timeline
    {
        //! Timer modes.
        enum class TimerMode
        {
            System,
            Audio,

            Count,
            First = System
        };
        TLRENDER_ENUM(TimerMode);
        TLRENDER_ENUM_SERIALIZE(TimerMode);

        //! External time mode.
        enum class ExternalTimeMode
        {
            Relative,
            Absolute,

            Count,
            First = Relative
        };
        TLRENDER_ENUM(ExternalTimeMode);
        TLRENDER_ENUM_SERIALIZE(ExternalTimeMode);

        //! Timeline player cache options.
        struct PlayerCacheOptions
        {
            //! Cache read ahead.
            otime::RationalTime readAhead = otime::RationalTime(2.0, 1.0);

            //! Cache read behind.
            otime::RationalTime readBehind = otime::RationalTime(0.5, 1.0);

            bool operator == (const PlayerCacheOptions&) const;
            bool operator != (const PlayerCacheOptions&) const;
        };

        //! Timeline player options.
        struct PlayerOptions
        {
            //! Cache options.
            PlayerCacheOptions cache;

            //! Timer mode.
            TimerMode timerMode = TimerMode::System;

            //! Audio channel count.
            size_t audioChannelCount = 2;

            //! Audio buffer frame count.
            size_t audioBufferFrameCount = 2048;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout = std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::milliseconds sleepTimeout = std::chrono::milliseconds(5);

            //! Current time.
            otime::RationalTime currentTime = time::invalidTime;

            //! External time mode.
            ExternalTimeMode externalTimeMode = ExternalTimeMode::Relative;

            bool operator == (const PlayerOptions&) const;
            bool operator != (const PlayerOptions&) const;
        };

        //! Get an external time from a source time.
        otime::RationalTime getExternalTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& externalTimeRange,
            ExternalTimeMode);
    }
}

#include <tlTimeline/PlayerOptionsInline.h>
