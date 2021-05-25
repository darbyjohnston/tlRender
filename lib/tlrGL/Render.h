// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrGL/FontSystem.h>

#include <tlrCore/BBox.h>
#include <tlrCore/Cache.h>

#include <glad.h>

namespace tlr
{
    namespace imaging
    {
        class Color4f;
        class Image;
        class Size;
    }

    namespace gl
    {
        class Shader;
        class Texture;

        //! OpenGL renderer.
        class Render : public std::enable_shared_from_this<Render>
        {
            TLR_NON_COPYABLE(Render);

        protected:
            void _init();
            Render();

        public:
            ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create();

            //! Start a render.
            void begin(const imaging::Size&);

            //! Finish a render.
            void end();

            //! Draw a rectangle.
            void drawRect(const math::BBox2f&, const imaging::Color4f&);

            //! Draw an image.
            void drawImage(const std::shared_ptr<imaging::Image>&, const math::BBox2f&);

            //! Draw text.
            void drawText(
                const std::vector<std::shared_ptr<Glyph> >& glyphs,
                const math::Vector2f& position,
                const imaging::Color4f&);

        private:
            std::shared_ptr<Shader> _shader;
            memory::Cache<GlyphInfo, std::shared_ptr<Texture> > _glyphTextureCache;
        };
    }
}