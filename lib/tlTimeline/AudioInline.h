// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool isTimeEqual(const AudioData& a, const AudioData& b)
        {
            return a.seconds == b.seconds;
        }
    }
}
