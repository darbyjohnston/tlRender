// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace geom
    {
        inline int edge(const glm::vec2& p, const glm::vec2& v0, const glm::vec2& v1)
        {
            return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y) * (v1.x - v0.x);
        }
    }
}
