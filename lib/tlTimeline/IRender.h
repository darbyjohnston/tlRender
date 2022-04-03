// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <tlCore/BBox.h>
#include <tlCore/Color.h>
#include <tlCore/Matrix.h>

namespace tl
{
    namespace imaging
    {
        struct ColorConfig;
        struct Glyph;
        class Image;
        class Size;
    }

    namespace timeline
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
        TLRENDER_ENUM(YUVRange);
        TLRENDER_ENUM_SERIALIZE(YUVRange);

        //! Channels.
        enum class Channels
        {
            Color,
            Red,
            Green,
            Blue,
            Alpha,

            Count,
            First = Color
        };
        TLRENDER_ENUM(Channels);
        TLRENDER_ENUM_SERIALIZE(Channels);

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
        TLRENDER_ENUM(AlphaBlend);
        TLRENDER_ENUM_SERIALIZE(AlphaBlend);

        //! Color values.
        struct Color
        {
        public:
            math::Vector3f add = math::Vector3f(0.F, 0.F, 0.F);
            math::Vector3f brightness = math::Vector3f(1.F, 1.F, 1.F);
            math::Vector3f contrast = math::Vector3f(1.F, 1.F, 1.F);
            math::Vector3f saturation = math::Vector3f(1.F, 1.F, 1.F);
            float tint = 0.F;
            bool invert = false;

            bool operator == (const Color&) const;
            bool operator != (const Color&) const;
        };

        //! Get a brightness color matrix.
        math::Matrix4x4f brightness(const math::Vector3f&);

        //! Get a contrast color matrix.
        math::Matrix4x4f contrast(const math::Vector3f&);

        //! Get a saturation color matrix.
        math::Matrix4x4f saturation(const math::Vector3f&);

        //! Get a tint color matrix.
        math::Matrix4x4f tint(float);

        //! Get a color matrix.
        math::Matrix4x4f color(const Color&);

        //! Levels values.
        struct Levels
        {
            float inLow = 0.F;
            float inHigh = 1.F;
            float gamma = 1.F;
            float outLow = 0.F;
            float outHigh = 1.F;

            bool operator == (const Levels&) const;
            bool operator != (const Levels&) const;
        };

        //! Exposure values.
        struct Exposure
        {
            float exposure = 0.F;
            float defog = 0.F;
            float kneeLow = 0.F;
            float kneeHigh = 5.F;

            bool operator == (const Exposure&) const;
            bool operator != (const Exposure&) const;
        };

        //! Image options.
        struct ImageOptions
        {
            YUVRange        yuvRange = YUVRange::FromFile;
            Channels        channels = Channels::Color;
            //! \todo Implement alpha blending options.
            AlphaBlend      alphaBlend = AlphaBlend::Straight;
            imaging::Mirror mirror;
            bool            colorEnabled = false;
            Color           color;
            bool            levelsEnabled = false;
            Levels          levels;
            bool            exposureEnabled = false;
            Exposure        exposure;
            bool            softClipEnabled = false;
            float           softClip = 0.F;

            bool operator == (const ImageOptions&) const;
            bool operator != (const ImageOptions&) const;
        };

        //! Comparison mode.
        enum class CompareMode
        {
            A,
            B,
            Wipe,
            Overlay,
            Difference,
            Horizontal,
            Vertical,
            Tile,

            Count,
            First = A
        };
        TLRENDER_ENUM(CompareMode);
        TLRENDER_ENUM_SERIALIZE(CompareMode);

        //! Comparison options.
        struct CompareOptions
        {
            CompareMode mode = CompareMode::A;
            math::Vector2f wipeCenter = math::Vector2f(.5F, .5F);
            float wipeRotation = 0.F;
            float overlay = .5F;

            bool operator == (const CompareOptions&) const;
            bool operator != (const CompareOptions&) const;
        };

        //! Get the bouncding boxes for the given compare mode and size.
        std::vector<math::BBox2i> tiles(CompareMode, const std::vector<imaging::Size>&);

        //! Get the render size for the given compare mode and sizes.
        imaging::Size getRenderSize(CompareMode, const std::vector<imaging::Size>&);

        //! Base class for renderers.
        class IRender : public std::enable_shared_from_this<IRender>
        {
        protected:
            void _init(const std::shared_ptr<system::Context>&);
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
            std::weak_ptr<system::Context> _context;
        };
    }
}

#include <tlTimeline/IRenderInline.h>
