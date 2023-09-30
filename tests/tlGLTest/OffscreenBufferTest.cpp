// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLTest/OffscreenBufferTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/OffscreenBuffer.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        OffscreenBufferTest::OffscreenBufferTest(const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::OffscreenBufferTest", context)
        {}

        std::shared_ptr<OffscreenBufferTest> OffscreenBufferTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<OffscreenBufferTest>(new OffscreenBufferTest(context));
        }

        void OffscreenBufferTest::run()
        {
            auto window = GLFWWindow::create(
                "OffscreenBufferTest",
                math::Size2i(1, 1),
                _context,
                static_cast<int>(GLFWWindowOptions::MakeCurrent));
            _enums();
            _buffer();
        }

        void OffscreenBufferTest::_enums()
        {
        }

        void OffscreenBufferTest::_buffer()
        {
        }
    }
}
