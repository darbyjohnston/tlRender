// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace imaging
    {
        inline Color4f::Color4f()
        {}

        inline Color4f::Color4f(float r, float g, float b, float a) :
            r(r), g(g), b(b), a(a)
        {}

        inline uint8_t fToU8(float value)
        {
            return static_cast<uint8_t>(std::min(std::max(value * 255.F, 0.F), 255.F));
        }
    }
}
