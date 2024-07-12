// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Util.h>
#include <tlCore/Vector.h>

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
        TLRENDER_ENUM(HDR_EOTF);
        TLRENDER_ENUM_SERIALIZE(HDR_EOTF);

        //! HDR color primaries.
        enum HDRPrimaries
        {
            Red   = 0,
            Green = 1,
            Blue  = 2,
            White = 3,

            Count,
            First = Red
        };
        TLRENDER_ENUM(HDRPrimaries);
        TLRENDER_ENUM_SERIALIZE(HDRPrimaries);

        //! HDR data.
        struct HDRData
        {
            HDR_EOTF eotf = HDR_EOTF::SDR;

            //! Default Rec. 2020 color primaries (red, green, blue, white).
            std::array<math::Vector2f, HDRPrimaries::Count> primaries =
            {
                math::Vector2f(.708F,  .292F),
                math::Vector2f(.170F,  .797F),
                math::Vector2f(.131F,  .046F),
                math::Vector2f(.3127F, .3290F)
            };
            math::FloatRange displayMasteringLuminance = math::FloatRange(0.F, 1000.F);
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
