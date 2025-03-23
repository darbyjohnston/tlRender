// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/AudioSystem.h>
#include <tlCore/Time.h>

#include <dtk/core/Memory.h>

namespace tl
{
    namespace timeline
    {
        //! Timeline player cache options.
        struct PlayerCacheOptions
        {
            // Video cache size in gigabytes.
            float videoGB = 4.F;

            // Audio cache size in gigabytes.
            float audioGB = .5F;

            // Number of seconds to read behind the current frame.
            float readBehind = .5F;

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

            //! Maximum number of video requests.
            size_t videoRequestMax = 16;

            //! Maximum number of audio requests.
            size_t audioRequestMax = 16;

            //! Audio buffer frame count.
            size_t audioBufferFrameCount = 500;

            //! Timeout for muting the audio when playback stutters.
            std::chrono::milliseconds muteTimeout = std::chrono::milliseconds(500);

            //! Timeout to sleep each tick.
            std::chrono::milliseconds sleepTimeout = std::chrono::milliseconds(5);

            //! Current time to start at.
            OTIO_NS::RationalTime currentTime = time::invalidTime;

            bool operator == (const PlayerOptions&) const;
            bool operator != (const PlayerOptions&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const PlayerCacheOptions&);

        void from_json(const nlohmann::json&, PlayerCacheOptions&);

        ///@}
    }
}
