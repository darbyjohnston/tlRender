// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool VideoLayer::operator == (const VideoLayer& other) const
        {
            return
                image == other.image &&
                imageOptions == other.imageOptions &&
                primitives == other.primitives &&
                imageB == other.imageB &&
                imageOptionsB == other.imageOptionsB &&
                primitivesB == other.primitivesB &&
                transition == other.transition &&
                transitionValue == other.transitionValue;
        }

        inline bool VideoLayer::operator != (const VideoLayer& other) const
        {
            return !(*this == other);
        }

        inline bool VideoData::operator == (const VideoData& other) const
        {
            return
                time == other.time &&
                layers == other.layers &&
                displayOptions == other.displayOptions;
        }

        inline bool VideoData::operator != (const VideoData& other) const
        {
            return !(*this == other);
        }

        inline bool isTimeEqual(const VideoData& a, const VideoData& b)
        {
            return a.time == b.time;
        }
    }
}
