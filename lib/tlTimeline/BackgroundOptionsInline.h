// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
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
                checkersColor0 == other.checkersColor0 &&
                checkersColor1 == other.checkersColor1 &&
                checkersSize == other.checkersSize;
        }

        inline bool BackgroundOptions::operator != (const BackgroundOptions& other) const
        {
            return !(*this == other);
        }
    }
}
