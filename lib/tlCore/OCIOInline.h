// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace imaging
    {
        inline bool ColorConfig::operator == (const ColorConfig& other) const
        {
            return fileName == other.fileName &&
                input == other.input &&
                display == other.display &&
                view == other.view &&
                look == other.look;
        }

        inline bool ColorConfig::operator != (const ColorConfig& other) const
        {
            return !(*this == other);
        }
    }
}
