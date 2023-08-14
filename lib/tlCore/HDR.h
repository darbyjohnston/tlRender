// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Vector.h>

namespace tl
{
    namespace image
    {
        //! HDR data.
        struct HDRData
        {
            uint8_t eotf = 0;

            //! Default Rec. 2020 color primaries.
            math::Vector2f redPrimaries   = math::Vector2f(.708F,  .292F);
            math::Vector2f greenPrimaries = math::Vector2f(.170F,  .797F);
            math::Vector2f bluePrimaries  = math::Vector2f(.131F,  .046F);
            math::Vector2f whitePrimaries = math::Vector2f(.3127F, .3290F);

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
