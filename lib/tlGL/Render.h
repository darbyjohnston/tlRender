// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace gl
    {
        class Shader;
        
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
            std::shared_ptr<Shader> getShader( const std::string& );
            void setTextureCacheSize(size_t) override;
            void setColorConfig(const timeline::ColorConfigOptions&) override;
            void setLUT(const timeline::LUTOptions&) override;
            void begin(const imaging::Size&) override;
            void end() override;
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
            void _drawVideo(
                const timeline::VideoData&,
                const math::BBox2i&,
                const std::shared_ptr<timeline::ImageOptions>&,
                const timeline::DisplayOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
