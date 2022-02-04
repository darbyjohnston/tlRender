// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <cstdint>

namespace tlr
{
    //! Imaging.
    namespace imaging
    {
        //! Color.
        class Color4f
        {
        public:
            Color4f();
            explicit Color4f(float r, float g, float b, float a = 1.F);

            float r, g, b, a;
        };

        //! Convert a floating point value to an 8-bit value.
        uint8_t fToU8(float);
    }
}

#include <tlrCore/ColorInline.h>
