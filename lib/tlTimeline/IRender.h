// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ColorConfigOptions.h>
#include <tlTimeline/CompareOptions.h>
#include <tlTimeline/LUTOptions.h>
#include <tlTimeline/RenderOptions.h>
#include <tlTimeline/Video.h>

#include <tlCore/Context.h>
#include <tlCore/FontSystem.h>
#include <tlCore/Mesh.h>

namespace tl
{
    namespace timeline
    {
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

            //! Set the color configuration. This function needs to be called before
            //! Render::begin().
            virtual void setColorConfig(const ColorConfigOptions&) = 0;

            //! Set the LUT. This function needs to be called before
            //! Render::begin().
            virtual void setLUT(const LUTOptions&) = 0;

            //! Start a render.
            virtual void begin(const imaging::Size&,
                const RenderOptions& = RenderOptions()) = 0;

            //! Finish a render.
            virtual void end() = 0;

            //! Set the view matrix.
            virtual void setView(const math::Matrix4x4f&) = 0;

            //! Draw a rectangle.
            virtual void drawRect(
                const math::BBox2i&,
                const imaging::Color4f&) = 0;

            //! Draw a triangle mesh.
            virtual void drawMesh(
                const geom::TriangleMesh2&,
                const imaging::Color4f&) = 0;

            //! Draw text.
            virtual void drawText(
                const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
                const math::Vector2i& position,
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
                const std::vector<math::BBox2i>&,
                const std::vector<ImageOptions>& = {},
                const std::vector<DisplayOptions>& = {},
                const CompareOptions& = CompareOptions()) = 0;

            //! Set whether the clipping rectangle is enabled.
            virtual void setClipRectEnabled(bool) = 0;

            //! Set the clipping rectangle.
            virtual void setClipRect(const math::BBox2i&) = 0;

        protected:
            std::weak_ptr<system::Context> _context;
            std::shared_ptr<imaging::FontSystem> _fontSystem;
        };
    }
}
