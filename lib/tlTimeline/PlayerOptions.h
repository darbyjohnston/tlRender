// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/AudioSystem.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace timeline
    {
        //! Timeline player cache options.
        struct PlayerCacheOptions
        {
            //! Cache read ahead.
            OTIO_NS::RationalTime readAhead = OTIO_NS::RationalTime(2.0, 1.0);

            //! Cache read behind.
            OTIO_NS::RationalTime readBehind = OTIO_NS::RationalTime(0.5, 1.0);

            bool operator == (const PlayerCacheOptions&) const;
            bool operator != (const PlayerCacheOptions&) const;
        };

        //! Timeline player options.
        struct PlayerOptions
        {
            //! Audio device index.
            audio::DeviceID audioDevice;

            //! Cache options.
            PlayerCacheOptions cache;

            //! Audio buffer frame count.
            size_t audioBufferFrameCount = 500;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout = std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::milliseconds sleepTimeout = std::chrono::milliseconds(5);

            //! Current time.
            OTIO_NS::RationalTime currentTime = time::invalidTime;

            bool operator == (const PlayerOptions&) const;
            bool operator != (const PlayerOptions&) const;
        };
    }
}
