// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace math
    {
        inline Matrix3x3f::Matrix3x3f()
        {
            v[0] = 1.F;
            v[1] = 0.F;
            v[2] = 0.F;

            v[3] = 0.F;
            v[4] = 1.F;
            v[5] = 0.F;

            v[6] = 0.F;
            v[7] = 0.F;
            v[8] = 1.F;
        }

        inline Matrix4x4f::Matrix4x4f()
        {
            v[ 0] = 1.F;
            v[ 1] = 0.F;
            v[ 2] = 0.F;
            v[ 3] = 0.F;

            v[ 4] = 0.F;
            v[ 5] = 1.F;
            v[ 6] = 0.F;
            v[ 7] = 0.F;

            v[ 8] = 0.F;
            v[ 9] = 0.F;
            v[10] = 1.F;
            v[11] = 0.F;

            v[12] = 0.F;
            v[13] = 0.F;
            v[14] = 0.F;
            v[15] = 1.F;
        }

        inline Matrix4x4f ortho(
            float left,
            float right,
            float bottom,
            float top,
            float near,
            float far)
        {
            Matrix4x4f m;
            
            m.v[0] = 2.F / (right - left);
            m.v[1] = 0.F;
            m.v[2] = 0.F;
            m.v[3] = 0.F;
            
            m.v[4] = 0.F;
            m.v[5] = 2.F / (top - bottom);
            m.v[6] = 0.F;
            m.v[7] = 0.F;
            
            m.v[8] = 0.F;
            m.v[9] = 0.F;
            m.v[10] = -2.F / (far - near);
            m.v[11] = 0.F;
            
            m.v[12] = -(right + left) / (right - left);
            m.v[13] = -(top + bottom) / (top - bottom);
            m.v[14] = -(far + near) / (far - near);
            m.v[15] = 1.F;
            
            return m;
        }
    }
}
