// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

namespace tlr
{
    namespace math
    {
        template<typename T>
        inline T clamp(T value, T min, T max)
        {
            return std::min(std::max(value, min), max);
        }
    }
}
