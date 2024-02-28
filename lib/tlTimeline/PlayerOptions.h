// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
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

        //! Compare time mode.
        enum class CompareTimeMode
        {
            Relative,
            Absolute,

            Count,
            First = Relative
        };
        TLRENDER_ENUM(CompareTimeMode);
        TLRENDER_ENUM_SERIALIZE(CompareTimeMode);

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

            //! Audio buffer frame count.
            size_t audioBufferFrameCount = 2048;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout = std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::milliseconds sleepTimeout = std::chrono::milliseconds(5);

            //! Current time.
            otime::RationalTime currentTime = time::invalidTime;

            //! Compare time mode.
            CompareTimeMode compareTimeMode = CompareTimeMode::Relative;

            bool operator == (const PlayerOptions&) const;
            bool operator != (const PlayerOptions&) const;
        };

        //! Get a compare time.
        otime::RationalTime getCompareTime(
            const otime::RationalTime& sourceTime,
            const otime::TimeRange& sourceTimeRange,
            const otime::TimeRange& compareTimeRange,
            CompareTimeMode);
    }
}

#include <tlTimeline/PlayerOptionsInline.h>
