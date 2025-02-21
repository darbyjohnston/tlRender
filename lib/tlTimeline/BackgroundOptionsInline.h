// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool BackgroundOptions::operator == (const BackgroundOptions& other) const
        {
            return
                type == other.type &&
                solidColor == other.solidColor &&
                checkersColor == other.checkersColor &&
                checkersSize == other.checkersSize &&
                gradientColor == other.gradientColor;
        }

        inline bool BackgroundOptions::operator != (const BackgroundOptions& other) const
        {
            return !(*this == other);
        }
    }
}
