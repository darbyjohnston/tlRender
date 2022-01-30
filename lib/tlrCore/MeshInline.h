// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace geom
    {
        inline float edge(const math::Vector2f& p, const math::Vector2f& v0, const math::Vector2f& v1)
        {
            return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y) * (v1.x - v0.x);
        }
    }
}
