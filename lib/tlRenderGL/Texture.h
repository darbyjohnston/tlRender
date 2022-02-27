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
        GLenum getTextureFormat(core::imaging::PixelType);

        //! Get the OpenGL internal texture format.
        GLenum getTextureInternalFormat(core::imaging::PixelType);

        //! Get the OpenGL texture type.
        GLenum getTextureType(core::imaging::PixelType);

        //! OpenGL texture.
        class Texture : public std::enable_shared_from_this<Texture>
        {
            TLRENDER_NON_COPYABLE(Texture);

        protected:
            void _init(
                const core::imaging::Info&,
                GLenum filterMin = GL_LINEAR,
                GLenum filterMag = GL_LINEAR);
            Texture();

        public:
            ~Texture();

            //! Create a new texture.
            static std::shared_ptr<Texture> create(
                const core::imaging::Info&,
                GLenum filterMin = GL_LINEAR,
                GLenum filterMag = GL_LINEAR);

            //! Get the OpenGL texture ID.
            GLuint getID() const;

            //! Get image information.
            const core::imaging::Info& getInfo() const;

            //! Set image information.
            void set(const core::imaging::Info&);

            //! \name Copy
            //! Copy image data to the texture.
            ///@{

            void copy(const core::imaging::Image&);
            void copy(const uint8_t*, const core::imaging::Info&);
            void copy(const core::imaging::Image&, uint16_t x, uint16_t y);

            ///@}

            //! Bind the texture.
            void bind();

        private:
            core::imaging::Info _info;
            GLenum _filterMin = GL_LINEAR;
            GLenum _filterMag = GL_LINEAR;
            GLuint _id = 0;
        };
    }
}
