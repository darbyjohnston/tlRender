// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>
#include <tlCore/Range.h>

namespace tl
{
    namespace gl
    {
        //! Texture atlas item.
        struct TextureAtlasItem
        {
            uint16_t w = 0;
            uint16_t h = 0;
            uint8_t textureIndex = 0;
            math::FloatRange textureU;
            math::FloatRange textureV;
        };

        //! Texture atlas item ID.
        typedef uint64_t TextureAtlasID;

        //! OpenGL texture atlas.
        class TextureAtlas : public std::enable_shared_from_this<TextureAtlas>
        {
            TLRENDER_NON_COPYABLE(TextureAtlas);

        protected:
            void _init(
                size_t textureCount,
                imaging::SizeType textureSize,
                imaging::PixelType textureType,
                timeline::ImageFilter filter,
                int border);

            TextureAtlas();

        public:
            ~TextureAtlas();

            //! Create a new texture atlas.
            static std::shared_ptr<TextureAtlas> create(
                size_t textureCount,
                imaging::SizeType textureSize,
                imaging::PixelType textureType,
                timeline::ImageFilter filter = timeline::ImageFilter::Linear,
                int border = 1);

            size_t getTextureCount() const;
            imaging::SizeType getTextureSize() const;
            imaging::PixelType getTextureType() const;
            std::vector<unsigned int> getTextures() const;

            bool getItem(TextureAtlasID, TextureAtlasItem&);

            TextureAtlasID addItem(
                const std::shared_ptr<imaging::Image>&,
                TextureAtlasItem&);

            float getPercentageUsed() const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}