// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool isTimeEqual(const VideoData& a, const VideoData& b)
        {
            return a.time == b.time;
        }
    }
}
