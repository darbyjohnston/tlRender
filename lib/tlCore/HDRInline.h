// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace image
    {
        inline bool HDRData::operator == (const HDRData& other) const
        {
            return
                other.eotf == eotf &&
                other.redPrimaries == redPrimaries &&
                other.greenPrimaries == greenPrimaries &&
                other.bluePrimaries == bluePrimaries &&
                other.whitePrimaries == whitePrimaries &&
                other.displayMasteringLuminance == displayMasteringLuminance &&
                other.maxCLL == maxCLL &&
                other.maxFALL == maxFALL;
        }

        inline bool HDRData::operator != (const HDRData& other) const
        {
            return !(other == *this);
        }
    }
}
