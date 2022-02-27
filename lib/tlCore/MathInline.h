// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <cmath>

namespace tl
{
    namespace core
    {
        namespace math
        {
            constexpr float deg2rad(float value) noexcept
            {
                return value / 360.F * pi2;
            }

            constexpr float rad2deg(float value) noexcept
            {
                return value / pi2 * 360.F;
            }

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

            inline size_t digits(int value) noexcept
            {
                size_t out = 0;
                if (value)
                {
                    while (value)
                    {
                        value /= 10;
                        ++out;
                    }
                }
                else
                {
                    out = 1;
                }
                return out;
            }

            inline bool fuzzyCompare(double a, double b, double e) noexcept
            {
                return fabs(a - b) < e;
            }

            inline bool fuzzyCompare(float a, float b, float e) noexcept
            {
                return fabsf(a - b) < e;
            }
        }
    }
}
