// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <tlGL/Texture.h>

#include <dtk/core/LRUCache.h>

namespace tl
{
    //! Timeline OpenGL support
    namespace timeline_gl
    {
        //! Texture cache.
        typedef dtk::LRUCache<
            std::shared_ptr<image::Image>,
            std::vector<std::shared_ptr<gl::Texture> > > TextureCache;

        //! OpenGL renderer.
        class Render : public timeline::IRender
        {
            TLRENDER_NON_COPYABLE(Render);

        protected:
            Render(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<TextureCache>&);

        public:
            virtual ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<TextureCache>& = nullptr);

            //! Get the texture cache.
            const std::shared_ptr<TextureCache>& getTextureCache() const;

            void begin(
                const math::Size2i&,
                const timeline::RenderOptions& = timeline::RenderOptions()) override;
            void end() override;

            math::Size2i getRenderSize() const override;
            void setRenderSize(const math::Size2i&) override;
            math::Box2i getViewport() const override;
            void setViewport(const math::Box2i&) override;
            void clearViewport(const dtk::Color4F&) override;
            bool getClipRectEnabled() const override;
            void setClipRectEnabled(bool) override;
            math::Box2i getClipRect() const override;
            void setClipRect(const math::Box2i&) override;
            math::Matrix4x4f getTransform() const override;
            void setTransform(const math::Matrix4x4f&) override;
            void setOCIOOptions(const timeline::OCIOOptions&) override;
            void setLUTOptions(const timeline::LUTOptions&) override;

            void drawRect(
                const math::Box2i&,
                const dtk::Color4F&) override;
            void drawMesh(
                const geom::TriangleMesh2&,
                const math::Vector2i& position,
                const dtk::Color4F&) override;
            void drawColorMesh(
                const geom::TriangleMesh2&,
                const math::Vector2i& position,
                const dtk::Color4F&) override;
            void drawText(
                const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
                const math::Vector2i& position,
                const dtk::Color4F&) override;
            void drawTexture(
                unsigned int,
                const math::Box2i&,
                const dtk::Color4F& = dtk::Color4F(1.F, 1.F, 1.F)) override;
            void drawImage(
                const std::shared_ptr<image::Image>&,
                const math::Box2i&,
                const dtk::Color4F& = dtk::Color4F(1.F, 1.F, 1.F),
                const timeline::ImageOptions& = timeline::ImageOptions()) override;
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions(),
                const timeline::BackgroundOptions& = timeline::BackgroundOptions()) override;

        private:
            void _displayShader();

            void _drawBackground(
                const std::vector<math::Box2i>&,
                const timeline::BackgroundOptions&);
            void _drawVideoA(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoB(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoWipe(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoOverlay(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoDifference(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideoTile(
                const std::vector<timeline::VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&);
            void _drawVideo(
                const timeline::VideoData&,
                const math::Box2i&,
                const std::shared_ptr<timeline::ImageOptions>&,
                const timeline::DisplayOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
