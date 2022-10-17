// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/Util.h>

namespace tl
{
    namespace view
    {
        void drawBorder(
            const math::BBox2i& bbox,
            int width,
            const imaging::Color4f& color,
            const std::shared_ptr<timeline::IRender>& render)
        {
            geom::TriangleMesh2 mesh;

            mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.min.y));
            mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.min.y));
            mesh.v.push_back(math::Vector2f(bbox.max.x + 1, bbox.max.y + 1));
            mesh.v.push_back(math::Vector2f(bbox.min.x, bbox.max.y + 1));
            mesh.v.push_back(math::Vector2f(bbox.min.x + width, bbox.min.y + width));
            mesh.v.push_back(math::Vector2f(bbox.max.x + 1 - width, bbox.min.y + width));
            mesh.v.push_back(math::Vector2f(bbox.max.x + 1 - width, bbox.max.y + 1 - width));
            mesh.v.push_back(math::Vector2f(bbox.min.x + width, bbox.max.y + 1 - width));

            mesh.triangles.push_back(geom::Triangle2({ 1, 2, 5 }));
            mesh.triangles.push_back(geom::Triangle2({ 2, 6, 5 }));
            mesh.triangles.push_back(geom::Triangle2({ 2, 3, 6 }));
            mesh.triangles.push_back(geom::Triangle2({ 3, 7, 6 }));
            mesh.triangles.push_back(geom::Triangle2({ 3, 4, 7 }));
            mesh.triangles.push_back(geom::Triangle2({ 4, 8, 7 }));
            mesh.triangles.push_back(geom::Triangle2({ 4, 1, 8 }));
            mesh.triangles.push_back(geom::Triangle2({ 1, 5, 8 }));

            render->drawMesh(mesh, color);
        }
    }
}
