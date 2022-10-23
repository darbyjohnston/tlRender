// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace gl
    {
        //! Get the OpenGL texture format.
        unsigned int getTextureFormat(imaging::PixelType);

        //! Get the OpenGL internal texture format.
        unsigned int getTextureInternalFormat(imaging::PixelType);

        //! Get the OpenGL texture type.
        unsigned int getTextureType(imaging::PixelType);

        //! OpenGL texture options.
        struct TextureOptions
        {
            timeline::ImageFilters filters;
            bool pbo = false;
        };

        //! Get the OpenGL texture filter.
        unsigned int getTextureFilter(timeline::ImageFilter);

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
            unsigned int getID() const;

            //! Get the image information.
            const imaging::Info& getInfo() const;

            //! Get the size.
            const imaging::Size& getSize() const;

            //! Get the pixel type.
            imaging::PixelType getPixelType() const;

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
            TLRENDER_PRIVATE();
        };
    }
}
