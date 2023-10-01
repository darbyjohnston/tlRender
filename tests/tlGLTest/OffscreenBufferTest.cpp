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
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "OffscreenBufferTest",
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
                _enums();
                _buffer();
            }
        }

        void OffscreenBufferTest::_enums()
        {
            _enum<OffscreenDepth>("OffscreenDepth", getOffscreenDepthEnums);
            _enum<OffscreenStencil>("OffscreenStencil", getOffscreenStencilEnums);
            _enum<OffscreenSampling>("OffscreenSampling", getOffscreenSamplingEnums);
        }

        void OffscreenBufferTest::_buffer()
        {
            {
                OffscreenBufferOptions options;
                options.colorType = offscreenColorDefault;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != OffscreenBufferOptions());
            }
            try
            {
                OffscreenBufferOptions options;
                options.colorType = offscreenColorDefault;
                options.depth = OffscreenDepth::_24;
                options.stencil = OffscreenStencil::_8;
                options.sampling = OffscreenSampling::None;
                const math::Size2i size(100, 200);
                auto buffer = OffscreenBuffer::create(size, options);
                TLRENDER_ASSERT(buffer->getSize() == size);
                TLRENDER_ASSERT(buffer->getWidth() == size.w);
                TLRENDER_ASSERT(buffer->getHeight() == size.h);
                TLRENDER_ASSERT(buffer->getOptions() == options);
                TLRENDER_ASSERT(buffer->getID());
                TLRENDER_ASSERT(buffer->getColorID());
                buffer->bind();
                TLRENDER_ASSERT(!doCreate(buffer, size, options));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            for (auto depth : getOffscreenDepthEnums())
            {
                try
                {
                    OffscreenBufferOptions options;
                    options.colorType = offscreenColorDefault;
                    options.depth = depth;
                    const math::Size2i size(100, 200);
                    auto buffer = OffscreenBuffer::create(size, options);
                    TLRENDER_ASSERT(buffer->getSize() == size);
                    TLRENDER_ASSERT(buffer->getWidth() == size.w);
                    TLRENDER_ASSERT(buffer->getHeight() == size.h);
                    TLRENDER_ASSERT(buffer->getOptions() == options);
                    TLRENDER_ASSERT(buffer->getID());
                    TLRENDER_ASSERT(buffer->getColorID());
                    buffer->bind();
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
            for (auto sampling : getOffscreenSamplingEnums())
            {
                try
                {
                    OffscreenBufferOptions options;
                    options.colorType = offscreenColorDefault;
                    options.sampling = sampling;
                    const math::Size2i size(100, 200);
                    auto buffer = OffscreenBuffer::create(size, options);
                    TLRENDER_ASSERT(buffer->getSize() == size);
                    TLRENDER_ASSERT(buffer->getWidth() == size.w);
                    TLRENDER_ASSERT(buffer->getHeight() == size.h);
                    TLRENDER_ASSERT(buffer->getOptions() == options);
                    TLRENDER_ASSERT(buffer->getID());
                    TLRENDER_ASSERT(buffer->getColorID());
                    buffer->bind();
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }
    }
}
