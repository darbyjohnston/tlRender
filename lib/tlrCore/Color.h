// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <algorithm>
#include <cstdint>

namespace tlr
{
    //! Imaging.
    namespace imaging
    {
        //! Color.
        struct Color4f
        {
            Color4f();
            explicit Color4f(float r, float g, float b, float a = 1.F);

            float r = 0.F;
            float g = 0.F;
            float b = 0.F;
            float a = 0.F;
        };

        //! Convert a floating point value to an 8-bit value.
        uint8_t fToU8(float);
    }
}

#include <tlrCore/ColorInline.h>
