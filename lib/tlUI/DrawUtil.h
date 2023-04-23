// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Mesh.h>

namespace tl
{
    namespace ui
    {
        //! Get a lighter color.
        imaging::Color4f lighter(const imaging::Color4f&, float);

        //! Get a darker color.
        imaging::Color4f darker(const imaging::Color4f&, float);

        //! Create a mesh for drawing a rectangle.
        geom::TriangleMesh2 rect(
            const math::BBox2i&,
            int cornerRadius = 0,
            size_t resolution = 8);

        //! Create a mesh for drawing a circle.
        geom::TriangleMesh2 circle(
            const math::Vector2i&,
            int radius = 0,
            size_t resolution = 120);

        //! Create a mesh for drawing a border.
        geom::TriangleMesh2 border(
            const math::BBox2i&,
            int width,
            int radius = 0,
            size_t resolution = 8);
    }
}
