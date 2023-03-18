// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        imaging::Color4f lighter(const imaging::Color4f& color, float amount)
        {
            return imaging::Color4f(
                color.r + amount,
                color.g + amount,
                color.b + amount,
                color.a);
        }

        imaging::Color4f darker(const imaging::Color4f& color, float amount)
        {
            return imaging::Color4f(
                color.r - amount,
                color.g - amount,
                color.b - amount,
                color.a);
        }

        geom::TriangleMesh2 border(const math::BBox2i& bbox, int borderSize)
        {
            geom::TriangleMesh2 out;
            const int x = bbox.x();
            const int y = bbox.y();
            const int w = bbox.w();
            const int h = bbox.h();
            const int b = borderSize;
            out.v.push_back(math::Vector2f(x, y));
            out.v.push_back(math::Vector2f(x + w - 1, y));
            out.v.push_back(math::Vector2f(x + w - 1, y + h - 1));
            out.v.push_back(math::Vector2f(x, y + h - 1));
            out.v.push_back(math::Vector2f(x + b, y + b));
            out.v.push_back(math::Vector2f(x + w - 1 - b, y + b));
            out.v.push_back(math::Vector2f(x + w - 1 - b, y + h - 1 - b));
            out.v.push_back(math::Vector2f(x + b, y + h - 1 - b));
            out.triangles.push_back(geom::Triangle2({ 1, 2, 5 }));
            out.triangles.push_back(geom::Triangle2({ 2, 6, 5 }));
            out.triangles.push_back(geom::Triangle2({ 2, 3, 6 }));
            out.triangles.push_back(geom::Triangle2({ 3, 7, 6 }));
            out.triangles.push_back(geom::Triangle2({ 3, 4, 7 }));
            out.triangles.push_back(geom::Triangle2({ 4, 8, 7 }));
            out.triangles.push_back(geom::Triangle2({ 4, 1, 8 }));
            out.triangles.push_back(geom::Triangle2({ 1, 5, 8 }));
            return out;
        }
    }
}
