// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool PlayerOptions::operator == (const PlayerOptions& other) const
        {
            return timerMode == other.timerMode &&
                audioBufferFrameCount == other.audioBufferFrameCount &&
                muteTimeout == other.muteTimeout &&
                sleepTimeout == other.sleepTimeout;
        }

        inline bool PlayerOptions::operator != (const PlayerOptions& other) const
        {
            return !(*this == other);
        }
    }
}
