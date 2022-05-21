// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <cstdint>
#include <iostream>

namespace tl
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

            bool operator == (const Color4f&) const;
            bool operator != (const Color4f&) const;
        };

        //! Convert a floating point value to an 8-bit value.
        uint8_t fToU8(float);

        std::ostream& operator << (std::ostream&, const Color4f&);

        std::istream& operator >> (std::istream&, Color4f&);
    }
}

#include <tlCore/ColorInline.h>
