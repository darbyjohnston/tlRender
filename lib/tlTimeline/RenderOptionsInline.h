// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool RenderOptions::operator == (const RenderOptions& other) const
        {
            return clear == other.clear;
        }

        inline bool RenderOptions::operator != (const RenderOptions& other) const
        {
            return !(*this == other);
        }
    }
}
