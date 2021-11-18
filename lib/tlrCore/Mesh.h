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
        //! Vertex.
        struct Vertex
        {
            size_t v = 0;
            size_t t = 0;
            size_t n = 0;
        };

        //! Face.
        struct Face
        {
            std::vector<Vertex> v;
        };

        //! Triangle.
        struct Triangle
        {
            Vertex v0;
            Vertex v1;
            Vertex v2;
        };

        //! Triangle mesh.
        struct TriangleMesh
        {
            std::vector<glm::vec3> v;
            std::vector<glm::vec3> c;
            std::vector<glm::vec2> t;
            std::vector<glm::vec3> n;
            std::vector<Triangle> triangles;
        };

        //! Create a sphere triangle mesh.
        TriangleMesh createSphere(
            float radius,
            size_t xResolution,
            size_t yResolution);
    }
}
