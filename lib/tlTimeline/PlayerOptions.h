// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
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
            otime::RationalTime readAhead = otime::RationalTime(2.0, 1.0);

            //! Cache read behind.
            otime::RationalTime readBehind = otime::RationalTime(0.5, 1.0);

            bool operator == (const PlayerCacheOptions&) const;
            bool operator != (const PlayerCacheOptions&) const;
        };

        //! Timeline player options.
        struct PlayerOptions
        {
            //! Audio device index.
            audio::DeviceID audioDevice;

            //! Use the minimum preferred sample rate for multiple devices with the same name.
            //!
            //! \bug This is to workaround an issue with RtAudio and Bluetooth headsets
            //! on macOS. When the microphone is activated RtAudio shows the headset as
            //! two separate devices. The devices may show different preferred sample rates
            //! but only the minimum sample rate seems to work. This option uses the minimum
            //! preferred sample rate for devices with the same name, and can be disabled in case it
            //! has other side effects.
            bool audioMinPreferredSampleRate = true;

            //! Cache options.
            PlayerCacheOptions cache;

            //! Audio buffer frame count.
            size_t audioBufferFrameCount = 500;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout = std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::milliseconds sleepTimeout = std::chrono::milliseconds(5);

            //! Current time.
            otime::RationalTime currentTime = time::invalidTime;

            bool operator == (const PlayerOptions&) const;
            bool operator != (const PlayerOptions&) const;
        };
    }
}

#include <tlTimeline/PlayerOptionsInline.h>
