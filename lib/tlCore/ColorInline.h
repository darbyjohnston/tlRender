// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <algorithm>

namespace tl
{
    namespace imaging
    {
        inline Color4f::Color4f() :
            r(0.F), g(0.F), b(0.F), a(0.F)
        {}

        inline Color4f::Color4f(float r, float g, float b, float a) :
            r(r), g(g), b(b), a(a)
        {}

        inline bool Color4f::operator == (const Color4f& other) const
        {
            return
                r == other.r &&
                g == other.g &&
                b == other.b &&
                a == other.a;
        }

        inline bool Color4f::operator != (const Color4f& other) const
        {
            return !(*this == other);
        }

        inline uint8_t fToU8(float value)
        {
            return static_cast<uint8_t>(std::min(std::max(value * 255.F, 0.F), 255.F));
        }
    }
}
