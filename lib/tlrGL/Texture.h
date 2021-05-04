// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Image.h>

#include <glad.h>

namespace tlr
{
    namespace gl
    {
        //! Get the OpenGL texture format.
        GLenum getTextureFormat(imaging::PixelType);

        //! Get the OpenGL internal texture format.
        GLenum getTextureInternalFormat(imaging::PixelType);

        //! Get the OpenGL texture type.
        GLenum getTextureType(imaging::PixelType);

        //! OpenGL texture.
        class Texture : public std::enable_shared_from_this<Texture>
        {
            TLR_NON_COPYABLE(Texture);

        protected:
            void _init(
                const imaging::Info&,
                GLenum filterMin = GL_LINEAR,
                GLenum filterMag = GL_LINEAR);
            Texture();

        public:
            ~Texture();

            //! Create a new texture.
            static std::shared_ptr<Texture> create(
                const imaging::Info&,
                GLenum filterMin = GL_LINEAR,
                GLenum filterMag = GL_LINEAR);

            //! Get the OpenGL texture ID.
            GLuint getID() const;

            //! Get image information.
            const imaging::Info& getInfo() const;

            //! Set image information.
            void set(const imaging::Info&);

            //! \name Copy
            //! Copy image data to the texture.
            ///@{

            void copy(const imaging::Image&);
            void copy(const imaging::Image&, uint16_t x, uint16_t y);

            ///@}

            //! Bind the texture.
            void bind();

        private:
            imaging::Info _info;
            GLenum _filterMin = GL_LINEAR;
            GLenum _filterMag = GL_LINEAR;
            GLuint _id = 0;
        };
    }
}
