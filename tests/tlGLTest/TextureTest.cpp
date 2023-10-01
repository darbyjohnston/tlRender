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
                _texture();
            }
        }

        void TextureTest::_texture()
        {
            {
                TextureOptions options;
                options.pbo = true;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != TextureOptions());
            }
            for (bool pbo : { false, true })
            {
                const image::Info info(100, 200, image::PixelType::RGBA_U8);
                TextureOptions options;
                options.pbo = pbo;
                auto texture = Texture::create(info, options);
                TLRENDER_ASSERT(texture->getID());
                TLRENDER_ASSERT(texture->getInfo() == info);
                TLRENDER_ASSERT(texture->getSize() == info.size);
                TLRENDER_ASSERT(texture->getWidth() == info.size.w);
                TLRENDER_ASSERT(texture->getHeight() == info.size.h);
                TLRENDER_ASSERT(texture->getPixelType() == info.pixelType);
                auto image = image::Image::create(info);
                texture->copy(image);
                auto image2 = image::Image::create(50, 100, image::PixelType::RGBA_U8);
                texture->copy(image2, 50, 100);
                texture->copy(image->getData(), info);
                texture->bind();
            }
        }
    }
}
