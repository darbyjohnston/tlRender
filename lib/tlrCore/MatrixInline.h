// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace math
    {
        inline Matrix3x3f::Matrix3x3f()
        {}

        inline Matrix3x3f::Matrix3x3f(
            float v0, float v1, float v2,
            float v3, float v4, float v5,
            float v6, float v7, float v8)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;

            v[3] = v3;
            v[4] = v4;
            v[5] = v5;

            v[6] = v6;
            v[7] = v7;
            v[8] = v8;
        }

        inline Matrix4x4f::Matrix4x4f()
        {}

        inline Matrix4x4f::Matrix4x4f(
            float v0, float v1, float v2, float v3,
            float v4, float v5, float v6, float v7,
            float v8, float v9, float v10, float v11,
            float v12, float v13, float v14, float v15)
        {
            v[0] = v0;
            v[1] = v1;
            v[2] = v2;
            v[3] = v3;

            v[4] = v4;
            v[5] = v5;
            v[6] = v6;
            v[7] = v7;

            v[8] = v8;
            v[9] = v9;
            v[10] = v10;
            v[11] = v11;

            v[12] = v12;
            v[13] = v13;
            v[14] = v14;
            v[15] = v15;
        }

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

        inline Matrix3x3f operator * (const Matrix3x3f& a, const Matrix3x3f& b)
        {
            Matrix3x3f out;
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    float tmp = 0.F;
                    for (int k = 0; k < 3; k++)
                    {
                        tmp += b.v[i * 3 + k] * a.v[k * 3 + j];
                    }
                    out.v[i * 3 + j] = tmp;
                }
            }
            return out;
        }

        inline Matrix4x4f operator * (const Matrix4x4f& a, const Matrix4x4f& b)
        {
            Matrix4x4f out;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    float tmp = 0.F;
                    for (int k = 0; k < 4; k++)
                    {
                        tmp += b.v[i * 4 + k] * a.v[k * 4 + j];
                    }
                    out.v[i * 4 + j] = tmp;
                }
            }
            return out;
        }
    }
}
