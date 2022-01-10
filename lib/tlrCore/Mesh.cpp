// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrCore/Mesh.h>

#include <tlrCore/Math.h>

namespace tlr
{
    namespace geom
    {
        TriangleMesh createSphere(
            float radius,
            size_t xResolution,
            size_t yResolution)
        {
            TriangleMesh out;

            //! \bug Use only a single vertex at each pole.
            for (size_t v = 0; v <= yResolution; ++v)
            {
                const float v1 = static_cast<float>(v) / static_cast<float>(yResolution);

                for (size_t u = 0; u <= xResolution; ++u)
                {
                    const float u1 = static_cast<float>(u) / static_cast<float>(xResolution);
                    const float x = radius * sinf(v1 * math::pi) * cosf(u1 * math::pi2);
                    const float y = radius * cosf(v1 * math::pi);
                    const float z = radius * sinf(v1 * math::pi) * sinf(u1 * math::pi2);
                    out.v.push_back(glm::vec3(x, y, z));
                    out.t.push_back(glm::vec2(u1, 1.F - v1));
                }
            }

            Triangle triangle;
            for (size_t v = 0; v < yResolution; ++v)
            {
                for (size_t u = 0; u < xResolution; ++u)
                {
                    const size_t i = u + v * (xResolution + 1);
                    const size_t j = u + (v + 1) * (xResolution + 1);
                    triangle.v0.v = triangle.v0.t = j + 1 + 1;
                    triangle.v1.v = triangle.v1.t = j + 1;
                    triangle.v2.v = triangle.v2.t = i + 1;
                    out.triangles.push_back(triangle);
                    triangle.v0.v = triangle.v0.t = i + 1;
                    triangle.v1.v = triangle.v1.t = i + 1 + 1;
                    triangle.v2.v = triangle.v2.t = j + 1 + 1;
                    out.triangles.push_back(triangle);
                }
            }

            return out;
        }
    }
}