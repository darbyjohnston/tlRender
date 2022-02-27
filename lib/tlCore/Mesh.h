// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Vector.h>

#include <vector>

namespace tl
{
    //! Geometry.
    namespace geom
    {
        //! Two-dimensional vertex.
        struct Vertex2
        {
            size_t v = 0;
            size_t t = 0;
        };

        //! Three-dimensional vertex.
        struct Vertex3
        {
            size_t v = 0;
            size_t t = 0;
            size_t n = 0;
        };

        //! Two-dimensional triangle.
        struct Triangle2
        {
            Vertex2 v[3];
        };

        //! Three-dimensional triangle.
        struct Triangle3
        {
            Vertex3 v[3];
        };

        //! Two-dimensional triangle mesh.
        struct TriangleMesh2
        {
            std::vector<math::Vector2f> v;
            std::vector<math::Vector2f> c;
            std::vector<math::Vector2f> t;
            std::vector<Triangle2> triangles;
        };

        //! Three-dimensional triangle mesh.
        struct TriangleMesh3
        {
            std::vector<math::Vector3f> v;
            std::vector<math::Vector3f> c;
            std::vector<math::Vector2f> t;
            std::vector<math::Vector3f> n;
            std::vector<Triangle3> triangles;
        };

        //! Edge function.
        float edge(const math::Vector2f& p, const math::Vector2f& v0, const math::Vector2f& v1);

        //! Create a sphere triangle mesh.
        TriangleMesh3 createSphere(
            float radius,
            size_t xResolution,
            size_t yResolution);
    }
}

#include <tlCore/MeshInline.h>
