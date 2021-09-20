// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrGL/FontSystem.h>

#include <tlrCore/BBox.h>
#include <tlrCore/Color.h>
#include <tlrCore/Timeline.h>

#include <array>

namespace tlr
{
    namespace core
    {
        class Context;
    }

    namespace imaging
    {
        class Image;
        class Size;
    }

    namespace gl
    {
        class Shader;
        class Texture;

        //! OpenColorIO configuration.
        struct ColorConfig
        {
            std::string config;
            std::string input;
            std::string display;
            std::string view;
            std::string look;

            bool operator == (const ColorConfig&) const;
            bool operator != (const ColorConfig&) const;
        };

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
    
        //! OpenGL renderer.
        class Render : public std::enable_shared_from_this<Render>
        {
            TLR_NON_COPYABLE(Render);

        protected:
            void _init(const std::shared_ptr<core::Context>&);
            Render();

        public:
            ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(const std::shared_ptr<core::Context>&);

            //! Set the texture cache size. This function should be called before
            //! Render::begin().
            void setTextureCacheSize(size_t);

            //! Set the color configuration. This function should be called before
            //! Render::begin().
            void setColorConfig(const ColorConfig&);

            //! Start a render.
            void begin(const imaging::Size&);

            //! Finish a render.
            void end();

            //! Draw a rectangle.
            void drawRect(
                const math::BBox2f&,
                const imaging::Color4f&);

            //! Draw an image.
            void drawImage(
                const std::shared_ptr<imaging::Image>&,
                const math::BBox2f&,
                const imaging::Color4f& = imaging::Color4f(1.F, 1.F, 1.F),
                const ImageOptions& = ImageOptions());

            //! Draw a timeline frame.
            void drawFrame(
                const timeline::Frame&,
                const ImageOptions & = ImageOptions());

            //! Draw text.
            void drawText(
                const std::vector<std::shared_ptr<Glyph> >& glyphs,
                const math::Vector2f& position,
                const imaging::Color4f&);

        private:
            void _setColorConfig(const ColorConfig&);
            void _delColorConfig();

            TLR_PRIVATE();
        };
    }
}

#include <tlrGL/RenderInline.h>
