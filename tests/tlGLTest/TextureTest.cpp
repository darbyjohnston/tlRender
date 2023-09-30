// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLTest/TextureTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/Texture.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        TextureTest::TextureTest(const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::TextureTest", context)
        {}

        std::shared_ptr<TextureTest> TextureTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<TextureTest>(new TextureTest(context));
        }

        void TextureTest::run()
        {
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "TextureTest",
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
