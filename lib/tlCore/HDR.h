// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Vector.h>

namespace tl
{
    namespace imaging
    {
        //! HDR data.
        struct HDRData
        {
            uint8_t eotf = 0;
            math::Vector2f redPrimaries;
            math::Vector2f greenPrimaries;
            math::Vector2f bluePrimaries;
            math::Vector2f whitePrimaries;
            math::FloatRange displayMasteringLuminance = math::FloatRange(0.F, 0.F);
            float maxCLL = 0.F;
            float maxFALL = 0.F;

            bool operator == (const HDRData&) const noexcept;
            bool operator != (const HDRData&) const noexcept;
        };

        void to_json(nlohmann::json&, const HDRData&);

        void from_json(const nlohmann::json&, HDRData&);
    }
}

#include <tlCore/HDRInline.h>
