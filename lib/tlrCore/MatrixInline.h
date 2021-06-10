// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace math
    {
        inline Matrix4x4f ortho(
            float left,
            float right,
            float bottom,
            float top,
            float near_,
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
            m.v[10] = -2.F / (far - near_);
            m.v[11] = 0.F;
            
            m.v[12] = -(right + left) / (right - left);
            m.v[13] = -(top + bottom) / (top - bottom);
            m.v[14] = -(far + near_) / (far - near_);
            m.v[15] = 1.F;
            
            return m;
        }
    }
}
