// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace image
    {
        inline bool HDRData::operator == (const HDRData& other) const
        {
            return
                eotf == other.eotf &&
                primaries == other.primaries &&
                displayMasteringLuminance == other.displayMasteringLuminance &&
                maxCLL == other.maxCLL &&
                maxFALL == other.maxFALL;
        }

        inline bool HDRData::operator != (const HDRData& other) const
        {
            return !(other == *this);
        }
    }
}
