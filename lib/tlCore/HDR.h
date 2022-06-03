// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>

namespace tl
{
    namespace imaging
    {
        //! HDR metadata.
        struct HDR
        {
            uint8_t eotf = 0;
            std::pair<float, float> redPrimaries = std::make_pair(0.F, 0.F);
            std::pair<float, float> greenPrimaries = std::make_pair(0.F, 0.F);
            std::pair<float, float> bluePrimaries = std::make_pair(0.F, 0.F);
            std::pair<float, float> whitePrimaries = std::make_pair(0.F, 0.F);
            math::FloatRange displayMasteringLuminance = math::FloatRange(0.F, 0.F);
            float maxCLL = 0.F;
            float maxFALL = 0.F;

            bool operator == (const HDR&) const noexcept;
            bool operator != (const HDR&) const noexcept;
        };

        void to_json(nlohmann::json&, const HDR&);

        void from_json(const nlohmann::json&, HDR&);
    }
}

#include <tlCore/HDRInline.h>
