// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLTest/MeshTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/Mesh.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        MeshTest::MeshTest(const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::MeshTest", context)
        {}

        std::shared_ptr<MeshTest> MeshTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<MeshTest>(new MeshTest(context));
        }

        void MeshTest::run()
        {
            auto window = gl::GLFWWindow::create(
                "gl_tests::MeshTest",
                math::Size2i(1, 1),
                _context,
                static_cast<int>(gl::GLFWWindowOptions::MakeCurrent));
            _enums();
            _convert();
            _mesh();
        }

        void MeshTest::_enums()
        {
            _enum<VBOType>("VBOType", getVBOTypeEnums);
            for (auto i : getVBOTypeEnums())
            {
                _print(string::Format("{0} byte count: {1}").arg(getLabel(i)).arg(getByteCount(i)));
            }
        }

        void MeshTest::_convert()
        {
            for (auto type : {
                    VBOType::Pos2_F32,
                    VBOType::Pos2_F32_UV_U16,
                    VBOType::Pos2_F32_Color_F32
                })
            {
                auto mesh = geom::box(math::Box2f(0.F, 1.F, 2.F, 3.F));
                auto data = convert(mesh, type);
                TLRENDER_ASSERT(!data.empty());
            }
            for (auto type : {
                    VBOType::Pos3_F32,
                    VBOType::Pos3_F32_UV_U16,
                    VBOType::Pos3_F32_UV_U16_Normal_U10,
                    VBOType::Pos3_F32_UV_U16_Normal_U10_Color_U8,
                    VBOType::Pos3_F32_UV_F32_Normal_F32,
                    VBOType::Pos3_F32_UV_F32_Normal_F32_Color_F32,
                    VBOType::Pos3_F32_Color_U8
                })
            {
                auto data = convert(geom::sphere(10.F, 10, 10), type);
                TLRENDER_ASSERT(!data.empty());
            }
        }

        void MeshTest::_mesh()
        {
            auto vbo = VBO::create(4, VBOType::Pos2_F32);
            TLRENDER_ASSERT(4 == vbo->getSize());
            TLRENDER_ASSERT(VBOType::Pos2_F32 == vbo->getType());
            TLRENDER_ASSERT(vbo->getID());
            auto mesh = geom::box(math::Box2f(0.F, 1.F, 2.F, 3.F));
            auto data = convert(mesh, VBOType::Pos2_F32);
            vbo->copy(data);
            auto vao = VAO::create(VBOType::Pos2_F32, vbo->getID());
            TLRENDER_ASSERT(vao->getID());
            vao->bind();
            vao->draw(GL_TRIANGLES, 0, 4);
        }
    }
}
