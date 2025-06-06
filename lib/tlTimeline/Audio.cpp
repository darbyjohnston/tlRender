// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/Audio.h>

namespace tl
{
    namespace timeline
    {
        bool AudioLayer::operator == (const AudioLayer& other) const
        {
            return audio == other.audio;
        }

        bool AudioLayer::operator != (const AudioLayer& other) const
        {
            return !(*this == other);
        }

        bool AudioData::operator == (const AudioData& other) const
        {
            return seconds == other.seconds &&
                layers == other.layers;
        }

        bool AudioData::operator != (const AudioData& other) const
        {
            return !(*this == other);
        }

        bool isTimeEqual(const AudioData& a, const AudioData& b)
        {
            return a.seconds == b.seconds;
        }
    }
}
