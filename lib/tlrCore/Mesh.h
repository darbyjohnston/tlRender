// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace tlr
{
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
            std::vector<glm::vec2> v;
            std::vector<glm::vec2> c;
            std::vector<glm::vec2> t;
            std::vector<Triangle2> triangles;
        };

        //! Three-dimensional triangle mesh.
        struct TriangleMesh3
        {
            std::vector<glm::vec3> v;
            std::vector<glm::vec3> c;
            std::vector<glm::vec2> t;
            std::vector<glm::vec3> n;
            std::vector<Triangle3> triangles;
        };

        //! Edge function.
        int edge(const glm::vec2& p, const glm::vec2& v0, const glm::vec2& v1);

        //! Create a sphere triangle mesh.
        TriangleMesh3 createSphere(
            float radius,
            size_t xResolution,
            size_t yResolution);
    }
}

#include <tlrCore/MeshInline.h>
