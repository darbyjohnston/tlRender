// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Color.h>
#include <dtk/core/Image.h>
#include <dtk/core/Mesh.h>

namespace tl
{
    namespace ui
    {
        //! Create a mesh for drawing a rectangle.
        dtk::TriMesh2F rect(
            const dtk::Box2I&,
            int cornerRadius = 0,
            size_t resolution = 8);

        //! Create a mesh for drawing a circle.
        dtk::TriMesh2F circle(
            const dtk::V2I&,
            int radius = 0,
            size_t resolution = 120);

        //! Create a mesh for drawing a border.
        dtk::TriMesh2F border(
            const dtk::Box2I&,
            int width,
            int radius = 0,
            size_t resolution = 8);

        //! Create a mesh for drawing a shadow.
        dtk::TriMesh2F shadow(
            const dtk::Box2I&,
            int cornerRadius,
            const float alpha = .2F,
            size_t resolution = 8);
    }
}
