// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace geom
    {
        inline Vertex2::Vertex2()
        {}

        inline Vertex2::Vertex2(size_t v, size_t t) :
            v(v),
            t(t)
        {}

        inline Vertex3::Vertex3()
        {}

        inline Vertex3::Vertex3(size_t v, size_t t, size_t n) :
            v(v),
            t(t),
            n(n)
        {}

        inline float edge(const math::Vector2f& p, const math::Vector2f& v0, const math::Vector2f& v1)
        {
            return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y) * (v1.x - v0.x);
        }
    }
}
