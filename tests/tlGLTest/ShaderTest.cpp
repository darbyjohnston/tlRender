// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLTest/ShaderTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/Shader.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        ShaderTest::ShaderTest(const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::ShaderTest", context)
        {}

        std::shared_ptr<ShaderTest> ShaderTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ShaderTest>(new ShaderTest(context));
        }

        void ShaderTest::run()
        {
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "ShaderTest",
                    math::Size2i(1, 1),
                    _context,
                    static_cast<int>(GLFWWindowOptions::MakeCurrent));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            if (window)
            {
            }
        }
    }
}
