// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/PlayerOptions.h>

namespace tl
{
    namespace timeline
    {
        bool PlayerCacheOptions::operator == (const PlayerCacheOptions& other) const
        {
            return
                readAhead == other.readAhead &&
                readBehind == other.readBehind;
        }

        bool PlayerCacheOptions::operator != (const PlayerCacheOptions& other) const
        {
            return !(*this == other);
        }

        bool PlayerOptions::operator == (const PlayerOptions& other) const
        {
            return
                audioDevice == other.audioDevice &&
                cache == other.cache &&
                audioBufferFrameCount == other.audioBufferFrameCount &&
                muteTimeout == other.muteTimeout &&
                sleepTimeout == other.sleepTimeout &&
                currentTime == other.currentTime;
        }

        bool PlayerOptions::operator != (const PlayerOptions& other) const
        {
            return !(*this == other);
        }
    }
}
