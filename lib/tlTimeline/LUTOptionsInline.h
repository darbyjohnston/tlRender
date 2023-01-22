// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool LUTOptions::operator == (const LUTOptions& other) const
        {
            return
                fileName == other.fileName &&
                order == other.order;
        }

        inline bool LUTOptions::operator != (const LUTOptions& other) const
        {
            return !(*this == other);
        }
    }
}
