// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/BBox.h>
#include <tlrCore/Color.h>
#include <tlrCore/Timeline.h>

namespace tlr
{
    namespace imaging
    {
        struct ColorConfig;
        struct Glyph;
        class Image;
        class Size;
    }

    namespace render
    {
        //! YUV value range.
        enum class YUVRange
        {
            FromFile,
            Full,
            Video,

            Count,
            First = FromFile
        };
        TLR_ENUM(YUVRange);
        TLR_ENUM_SERIALIZE(YUVRange);

        //! Image channels display.
        enum class ImageChannelsDisplay
        {
            Color,
            Red,
            Green,
            Blue,
            Alpha,

            Count,
            First = Color
        };
        TLR_ENUM(ImageChannelsDisplay);
        TLR_ENUM_SERIALIZE(ImageChannelsDisplay);

        //! Alpha channel blending.
        //!
        //! References:
        //! - https://microsoft.github.io/Win2D/html/PremultipliedAlpha.htm
        enum class AlphaBlend
        {
            None,
            Straight,
            Premultiplied,

            Count,
            First = None
        };
        TLR_ENUM(AlphaBlend);
        TLR_ENUM_SERIALIZE(AlphaBlend);

        //! Image color values.
        struct ImageColor
        {
        public:
            math::Vector3f add        = math::Vector3f(0.F, 0.F, 0.F);
            math::Vector3f brightness = math::Vector3f(1.F, 1.F, 1.F);
            math::Vector3f contrast   = math::Vector3f(1.F, 1.F, 1.F);
            math::Vector3f saturation = math::Vector3f(1.F, 1.F, 1.F);
            float          tint       = 0.F;
            bool           invert     = false;

            bool operator == (const ImageColor&) const;
            bool operator != (const ImageColor&) const;
        };
        
        //! Image levels values.
        struct ImageLevels
        {
            float inLow   = 0.F;
            float inHigh  = 1.F;
            float gamma   = 1.F;
            float outLow  = 0.F;
            float outHigh = 1.F;

            bool operator == (const ImageLevels&) const;
            bool operator != (const ImageLevels&) const;
        };
        
        //! Image exposure values.
        struct ImageExposure
        {
            float exposure = 0.F;
            float defog    = 0.F;
            float kneeLow  = 0.F;
            float kneeHigh = 5.F;

            bool operator == (const ImageExposure&) const;
            bool operator != (const ImageExposure&) const;
        };

        //! Image options.
        struct ImageOptions
        {
            YUVRange             yuvRange        = YUVRange::FromFile;
            ImageChannelsDisplay channelsDisplay = ImageChannelsDisplay::Color;
            //! \todo Implement alpha blending options.
            AlphaBlend           alphaBlend      = AlphaBlend::Straight;
            imaging::Mirror      mirror;
            bool                 colorEnabled    = false;
            ImageColor           color;
            bool                 levelsEnabled   = false;
            ImageLevels          levels;
            bool                 exposureEnabled = false;
            ImageExposure        exposure;
            bool                 softClipEnabled = false;
            float                softClip        = 0.F;

            bool operator == (const ImageOptions&) const;
            bool operator != (const ImageOptions&) const;
        };

        //! Comparison mode.
        enum class CompareMode
        {
            A,
            B,
            Horizontal,
            Vertical,
            Free,
            Tiles,

            Count,
            First = A
        };
        TLR_ENUM(CompareMode);
        TLR_ENUM_SERIALIZE(CompareMode);

        //! Comparison options.
        struct CompareOptions
        {
            CompareMode mode = CompareMode::A;
            float horizontal = .5F;
            float vertical = .5F;
            math::Vector2i freePos;
            float freeRot = 0.F;

            bool operator == (const CompareOptions&) const;
            bool operator != (const CompareOptions&) const;
        };
    
        //! Base class for renderers.
        class IRender : public std::enable_shared_from_this<IRender>
        {
        protected:
            void _init(const std::shared_ptr<core::Context>&);
            IRender();

        public:
            virtual ~IRender() = 0;

            //! Set the texture cache size. This function should be called before
            //! Render::begin().
            virtual void setTextureCacheSize(size_t) = 0;

            //! Set the color configuration. This function should be called before
            //! Render::begin().
            virtual void setColorConfig(const imaging::ColorConfig&) = 0;

            //! Start a render.
            virtual void begin(const imaging::Size&) = 0;

            //! Finish a render.
            virtual void end() = 0;

            //! Draw a rectangle.
            virtual void drawRect(
                const math::BBox2i&,
                const imaging::Color4f&) = 0;

            //! Draw an image.
            virtual void drawImage(
                const std::shared_ptr<imaging::Image>&,
                const math::BBox2i&,
                const imaging::Color4f& = imaging::Color4f(1.F, 1.F, 1.F),
                const ImageOptions& = ImageOptions()) = 0;

            //! Draw timeline video data.
            virtual void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<ImageOptions>& = {},
                const CompareOptions& = CompareOptions()) = 0;

            //! Draw text.
            virtual void drawText(
                const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
                const math::Vector2i& position,
                const imaging::Color4f&) = 0;

        protected:
            std::weak_ptr<core::Context> _context;
        };
    }
}

#include <tlrCore/IRenderInline.h>
