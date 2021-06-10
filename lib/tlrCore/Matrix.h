// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

namespace tlr
{
    namespace math
    {
        //! 3x3 matrix.
        struct Matrix3x3f
        {
            float v[9] =
            {
                1.F, 0.F, 0.F,
                0.F, 1.F, 0.F,
                0.F, 0.F, 1.F
            };
        };

        //! 4x4 matrix.
        struct Matrix4x4f
        {
            float v[16] =
            {
                1.F, 0.F, 0.F, 0.F,
                0.F, 1.F, 0.F, 0.F,
                0.F, 0.F, 1.F, 0.F,
                0.F, 0.F, 0.F, 1.F
            };
        };

        //! Create an orthgraphic projection matrix.
        Matrix4x4f ortho(
            float left,
            float right,
            float bottom,
            float top,
            float near,
            float far);
    }
}

#include <tlrCore/MatrixInline.h>
