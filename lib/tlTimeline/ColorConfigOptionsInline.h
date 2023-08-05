// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool ColorConfigOptions::operator == (const ColorConfigOptions& other) const
        {
            return
                enabled == other.enabled &&
                fileName == other.fileName &&
                input == other.input &&
                display == other.display &&
                view == other.view &&
                look == other.look;
        }

        inline bool ColorConfigOptions::operator != (const ColorConfigOptions& other) const
        {
            return !(*this == other);
        }
    }
}
