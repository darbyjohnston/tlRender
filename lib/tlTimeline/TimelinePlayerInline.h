// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool PlayerCacheOptions::operator == (const PlayerCacheOptions& other) const
        {
            return
                videoByteCount == other.videoByteCount &&
                audioByteCount == other.audioByteCount &&
                readAhead == other.readAhead &&
                readBehind == other.readBehind;
        }

        inline bool PlayerCacheOptions::operator != (const PlayerCacheOptions& other) const
        {
            return !(*this == other);
        }

        inline bool PlayerCacheInfo::operator == (const PlayerCacheInfo& other) const
        {
            return
                videoPercentage == other.videoPercentage &&
                audioPercentage == other.audioPercentage &&
                videoFrames == other.videoFrames &&
                audioFrames == other.audioFrames;
        }

        inline bool PlayerCacheInfo::operator != (const PlayerCacheInfo& other) const
        {
            return !(*this == other);
        }

        inline bool PlayerOptions::operator == (const PlayerOptions& other) const
        {
            return
                cache == other.cache &&
                timerMode == other.timerMode &&
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
