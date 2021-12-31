// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace imaging
    {
        inline bool ColorConfig::operator == (const ColorConfig& other) const
        {
            return config == other.config &&
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