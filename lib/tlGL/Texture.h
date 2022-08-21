// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <tlGlad/gl.h>

namespace tl
{
    namespace gl
    {
        //! Get the OpenGL texture format.
        GLenum getTextureFormat(imaging::PixelType);

        //! Get the OpenGL internal texture format.
        GLenum getTextureInternalFormat(imaging::PixelType);

        //! Get the OpenGL texture type.
        GLenum getTextureType(imaging::PixelType);

        //! OpenGL texture options.
        struct TextureOptions
        {
            GLenum minifyFilter = GL_LINEAR;
            GLenum magnifyFilter = GL_LINEAR;
            bool pbo = false;
        };

        //! OpenGL texture.
        class Texture : public std::enable_shared_from_this<Texture>
        {
            TLRENDER_NON_COPYABLE(Texture);

        protected:
            void _init(
                const imaging::Info&,
                const TextureOptions& = TextureOptions());
            Texture();

        public:
            ~Texture();

            //! Create a new texture.
            static std::shared_ptr<Texture> create(
                const imaging::Info&,
                const TextureOptions& = TextureOptions());

            //! Get the OpenGL texture ID.
            GLuint getID() const;

            //! Get image information.
            const imaging::Info& getInfo() const;

            //! \name Copy
            //! Copy image data to the texture.
            ///@{

            void copy(const imaging::Image&);
            void copy(const uint8_t*, const imaging::Info&);
            void copy(const imaging::Image&, uint16_t x, uint16_t y);

            ///@}

            //! Bind the texture.
            void bind();

        private:
            imaging::Info _info;
            GLuint _pbo = 0;
            GLuint _id = 0;
        };
    }
}
