// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/Mesh.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace geom
    {
        TriangleMesh2 box(const math::Box2i& box, bool flipV)
        {
            TriangleMesh2 out;

            const auto& min = box.min;
            const auto& max = box.max;
            out.v.push_back(math::Vector2f(min.x, min.y));
            out.v.push_back(math::Vector2f(max.x + 1, min.y));
            out.v.push_back(math::Vector2f(max.x + 1, max.y + 1));
            out.v.push_back(math::Vector2f(min.x, max.y + 1));
            out.t.push_back(math::Vector2f(0.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 0.F : 1.F));
            out.t.push_back(math::Vector2f(0.F, flipV ? 0.F : 1.F));

            Triangle2 triangle;
            triangle.v[0].v = 1;
            triangle.v[1].v = 2;
            triangle.v[2].v = 3;
            triangle.v[0].t = 1;
            triangle.v[1].t = 2;
            triangle.v[2].t = 3;
            out.triangles.push_back(triangle);
            triangle.v[0].v = 3;
            triangle.v[1].v = 4;
            triangle.v[2].v = 1;
            triangle.v[0].t = 3;
            triangle.v[1].t = 4;
            triangle.v[2].t = 1;
            out.triangles.push_back(triangle);

            return out;
        }

        TriangleMesh2 box(const math::Box2f& box, bool flipV)
        {
            TriangleMesh2 out;

            const auto& min = box.min;
            const auto& max = box.max;
            out.v.push_back(math::Vector2f(min.x, min.y));
            out.v.push_back(math::Vector2f(max.x, min.y));
            out.v.push_back(math::Vector2f(max.x, max.y));
            out.v.push_back(math::Vector2f(min.x, max.y));
            out.t.push_back(math::Vector2f(0.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 1.F : 0.F));
            out.t.push_back(math::Vector2f(1.F, flipV ? 0.F : 1.F));
            out.t.push_back(math::Vector2f(0.F, flipV ? 0.F : 1.F));

            Triangle2 triangle;
            triangle.v[0].v = 1;
            triangle.v[1].v = 2;
            triangle.v[2].v = 3;
            triangle.v[0].t = 1;
            triangle.v[1].t = 2;
            triangle.v[2].t = 3;
            out.triangles.push_back(triangle);
            triangle.v[0].v = 3;
            triangle.v[1].v = 4;
            triangle.v[2].v = 1;
            triangle.v[0].t = 3;
            triangle.v[1].t = 4;
            triangle.v[2].t = 1;
            out.triangles.push_back(triangle);

            return out;
        }

        TriangleMesh3 sphere(
            float radius,
            size_t xResolution,
            size_t yResolution)
        {
            TriangleMesh3 out;

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
                    out.v.push_back(math::Vector3f(x, y, z));
                    out.t.push_back(math::Vector2f(u1, 1.F - v1));
                }
            }

            Triangle3 triangle;
            for (size_t v = 0; v < yResolution; ++v)
            {
                for (size_t u = 0; u < xResolution; ++u)
                {
                    const size_t i = u + v * (xResolution + 1);
                    const size_t j = u + (v + 1) * (xResolution + 1);
                    triangle.v[0].v = triangle.v[0].t = j + 1 + 1;
                    triangle.v[1].v = triangle.v[1].t = j + 1;
                    triangle.v[2].v = triangle.v[2].t = i + 1;
                    out.triangles.push_back(triangle);
                    triangle.v[0].v = triangle.v[0].t = i + 1;
                    triangle.v[1].v = triangle.v[1].t = i + 1 + 1;
                    triangle.v[2].v = triangle.v[2].t = j + 1 + 1;
                    out.triangles.push_back(triangle);
                }
            }

            return out;
        }
    }
}
