// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/Video.h>

namespace tl
{
    namespace timeline
    {
        bool isTimeEqual(const std::vector<VideoData>& a, const std::vector<VideoData>& b)
        {
            bool out = false;
            if (a.size() == b.size())
            {
                for (size_t i = 0; i < a.size(); ++i)
                {
                    out |= time::compareExact(a[i].time, b[i].time);
                    if (out)
                    {
                        break;
                    }
                }
            }
            else if (a.empty() && b.empty())
            {
                out = true;
            }
            return out;
        }
    }
}
