// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/Range.h>
#include <feather-tk/core/Vector.h>
#include <feather-tk/core/Util.h>

#include <nlohmann/json.hpp>

#include <array>

namespace tl
{
    namespace image
    {
        //! HDR EOTF.
        enum class HDR_EOTF
        {
            SDR,
            HDR,
            ST2084,

            Count,
            First = SDR
        };
        FEATHER_TK_ENUM(HDR_EOTF);

        //! HDR color primaries.
        enum class HDRPrimaries
        {
            Red,
            Green,
            Blue,
            White,

            Count,
            First = Red
        };
        FEATHER_TK_ENUM(HDRPrimaries);

        //! HDR data.
        struct HDRData
        {
            HDR_EOTF eotf = HDR_EOTF::SDR;

            //! Default Rec. 2020 color primaries (red, green, blue, white).
            std::array<feather_tk::V2F, static_cast<size_t>(HDRPrimaries::Count)> primaries =
            {
                feather_tk::V2F(.708F,  .292F),
                feather_tk::V2F(.170F,  .797F),
                feather_tk::V2F(.131F,  .046F),
                feather_tk::V2F(.3127F, .3290F)
            };
            feather_tk::RangeF displayMasteringLuminance = feather_tk::RangeF(0.F, 1000.F);
            float maxCLL  = 1000.F;
            float maxFALL = 400.F;

            bool operator == (const HDRData&) const;
            bool operator != (const HDRData&) const;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const HDRData&);

        void from_json(const nlohmann::json&, HDRData&);

        ///@}
    }
}

#include <tlCore/HDRInline.h>
