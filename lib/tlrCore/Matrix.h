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
            Matrix3x3f();
            Matrix3x3f(
                float v0, float v1, float v2,
                float v3, float v4, float v5,
                float v6, float v7, float v8);

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
            Matrix4x4f();
            Matrix4x4f(
                float v0, float v1, float v2, float v3,
                float v4, float v5, float v6, float v7,
                float v8, float v9, float v10, float v11,
                float v12, float v13, float v14, float v15);

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

        //! Multiply two 3x3 matrices.
        Matrix3x3f operator * (const Matrix3x3f&, const Matrix3x3f&);
        
        //! Multiply two 4x4 matrices.
        Matrix4x4f operator * (const Matrix4x4f&, const Matrix4x4f&);
    }
}

#include <tlrCore/MatrixInline.h>
