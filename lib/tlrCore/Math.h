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
    }
}

#include <tlrCore/MathInline.h>
