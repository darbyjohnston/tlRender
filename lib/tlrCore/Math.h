// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

namespace tlr
{
    //! Math.
    namespace math
    {
        //! Clamp a value.
        template<typename T>
        T clamp(T value, T min, T max);

        //! Linear interpolation.
        template<typename T, typename U>
        constexpr T lerp(U value, T min, T max) noexcept;

        //! Smooth step function.
        template<typename T>
        constexpr T smoothStep(T value, T min, T max) noexcept;
    }
}

#include <tlrCore/MathInline.h>
