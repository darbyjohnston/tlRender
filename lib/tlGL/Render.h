// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace gl
    {
        //! OpenGL renderer.
        class Render : public timeline::IRender
        {
            TLRENDER_NON_COPYABLE(Render);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            Render();

        public:
            ~Render() override;

            //! Create a new renderer.
            static std::shared_ptr<Render> create(const std::shared_ptr<system::Context>&);

            void begin(
                const imaging::Size&,
                const timeline::ColorConfigOptions& = timeline::ColorConfigOptions(),
                const timeline::LUTOptions& = timeline::LUTOptions(),
                const timeline::RenderOptions& = timeline::RenderOptions()) override;
            void end() override;

            void setViewport(const math::BBox2i&) override;
            void clearViewport(const imaging::Color4f&) override;
            void setClipRectEnabled(bool) override;
            void setClipRect(const math::BBox2i&) override;
            void setTransform(const math::Matrix4x4f&) override;

            void drawRect(
                const math::BBox2i&,
                const imaging::Color4f&) override;
            void drawMesh(
                const geom::TriangleMesh2&,
                const imaging::Color4f&) override;
            void drawText(
                const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
                const math::Vector2i& position,
                const imaging::Color4f&) override;
            void drawImage(
                const std::shared_ptr<imaging::Image>&,
                const math::BBox2i&,
                const imaging::Color4f& = imaging::Color4f(1.F, 1.F, 1.F),
                const timeline::ImageOptions& = timeline::ImageOptions()) override;
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::BBox2i>&,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions()) override;

        private:
            void _setColorConfig(const timeline::ColorConfigOptions&);
            void _setLUT(const timeline::LUTOptions&);
            void _drawVideo(
                const timeline::VideoData&,
                const math::BBox2i&,
                const std::shared_ptr<timeline::ImageOptions>&,
                const timeline::DisplayOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
