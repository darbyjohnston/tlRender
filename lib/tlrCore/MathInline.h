// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

namespace tlr
{
    namespace math
    {
        template<typename T>
        constexpr T clamp(T value, T min, T max)
        {
            return std::min(std::max(value, min), max);
        }

        template<class T, class U>
        constexpr T lerp(U value, T min, T max) noexcept
        {
            return min + T(value * (max - min));
        }

        constexpr float smoothStep(float value, float min, float max) noexcept
        {
            return lerp(value * value * (3.F - (2.F * value)), min, max);
        }

        constexpr double smoothStep(double value, double min, double max) noexcept
        {
            return lerp(value * value * (3. - (2. * value)), min, max);
        }
    }
}
