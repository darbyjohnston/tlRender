// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/Video.h>

namespace tl
{
    namespace timeline
    {
        bool VideoLayer::operator == (const VideoLayer& other) const
        {
            return
                image == other.image &&
                imageOptions == other.imageOptions &&
                imageB == other.imageB &&
                imageOptionsB == other.imageOptionsB &&
                transition == other.transition &&
                transitionValue == other.transitionValue;
        }

        bool VideoLayer::operator != (const VideoLayer& other) const
        {
            return !(*this == other);
        }

        bool VideoData::operator == (const VideoData& other) const
        {
            return
                size == other.size &&
                time.strictly_equal(other.time) &&
                layers == other.layers;
        }

        bool VideoData::operator != (const VideoData& other) const
        {
            return !(*this == other);
        }

        bool isTimeEqual(const VideoData& a, const VideoData& b)
        {
            return a.time.strictly_equal(b.time);
        }
    }
}
