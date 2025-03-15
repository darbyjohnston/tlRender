// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <dtk/gl/Render.h>
#include <dtk/core/LRUCache.h>

namespace tl
{
    //! Timeline OpenGL support
    namespace timeline_gl
    {
        //! OpenGL renderer.
        class Render : public timeline::IRender
        {
            DTK_NON_COPYABLE(Render);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::gl::TextureCache>&);

            Render();

        public:
            virtual ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::gl::TextureCache>& = nullptr);

            const std::shared_ptr<dtk::gl::TextureCache>& getTextureCache() const;

            void setOCIOOptions(const timeline::OCIOOptions&) override;
            void setLUTOptions(const timeline::LUTOptions&) override;

            void drawTexture(
                unsigned int,
                const dtk::Box2I&,
                const dtk::Color4F & = dtk::Color4F(1.F, 1.F, 1.F),
                dtk::AlphaBlend = dtk::AlphaBlend::Straight) override;
            void drawBackground(
                const std::vector<dtk::Box2I>&,
                const dtk::M44F&,
                const timeline::BackgroundOptions&) override;
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions(),
                dtk::ImageType colorBuffer = dtk::ImageType::RGBA_U8) override;
            void drawForeground(
                const std::vector<dtk::Box2I>&,
                const dtk::M44F&,
                const timeline::ForegroundOptions&) override;

            void begin(
                const dtk::Size2I&,
                const dtk::RenderOptions& = dtk::RenderOptions()) override;
            void end() override;
            dtk::Size2I getRenderSize() const override;
            void setRenderSize(const dtk::Size2I&) override;
            dtk::RenderOptions getRenderOptions() const override;
            dtk::Box2I getViewport() const override;
            void setViewport(const dtk::Box2I&) override;
            void clearViewport(const dtk::Color4F&) override;
            bool getClipRectEnabled() const override;
            void setClipRectEnabled(bool) override;
            dtk::Box2I getClipRect() const override;
            void setClipRect(const dtk::Box2I&) override;
            dtk::M44F getTransform() const override;
            void setTransform(const dtk::M44F&) override;
            void drawRect(
                const dtk::Box2F&,
                const dtk::Color4F&) override;
            void drawRects(
                const std::vector<dtk::Box2F>&,
                const dtk::Color4F&) override;
            void drawLine(
                const dtk::V2F&,
                const dtk::V2F&,
                const dtk::Color4F&,
                const dtk::LineOptions& = dtk::LineOptions()) override;
            void drawLines(
                const std::vector<std::pair<dtk::V2F, dtk::V2F> >&,
                const dtk::Color4F&,
                const dtk::LineOptions& = dtk::LineOptions()) override;
            void drawMesh(
                const dtk::TriMesh2F&,
                const dtk::Color4F& = dtk::Color4F(1.F, 1.F, 1.F, 1.F),
                const dtk::V2F& pos = dtk::V2F()) override;
            void drawColorMesh(
                const dtk::TriMesh2F&,
                const dtk::Color4F& = dtk::Color4F(1.F, 1.F, 1.F, 1.F),
                const dtk::V2F& pos = dtk::V2F()) override;
            void drawText(
                const std::vector<std::shared_ptr<dtk::Glyph> >&,
                const dtk::FontMetrics&,
                const dtk::V2F& position,
                const dtk::Color4F& = dtk::Color4F(1.F, 1.F, 1.F, 1.F)) override;
            void drawImage(
                const std::shared_ptr<dtk::Image>&,
                const dtk::TriMesh2F&,
                const dtk::Color4F& = dtk::Color4F(1.F, 1.F, 1.F, 1.F),
                const dtk::ImageOptions& = dtk::ImageOptions()) override;
            void drawImage(
                const std::shared_ptr<dtk::Image>&,
                const dtk::Box2F&,
                const dtk::Color4F& = dtk::Color4F(1.F, 1.F, 1.F, 1.F),
                const dtk::ImageOptions& = dtk::ImageOptions()) override;

        private:
            void _displayShader();

            void _drawVideoA(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                dtk::ImageType colorBuffer);
            void _drawVideoB(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                dtk::ImageType colorBuffer);
            void _drawVideoWipe(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                dtk::ImageType colorBuffer);
            void _drawVideoOverlay(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                dtk::ImageType colorBuffer);
            void _drawVideoDifference(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                dtk::ImageType colorBuffer);
            void _drawVideoTile(
                const std::vector<timeline::VideoData>&,
                const std::vector<dtk::Box2I>&,
                const std::vector<dtk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                dtk::ImageType colorBuffer);
            void _drawVideo(
                const timeline::VideoData&,
                const dtk::Box2I&,
                const std::shared_ptr<dtk::ImageOptions>&,
                const timeline::DisplayOptions&,
                dtk::ImageType colorBuffer);

            DTK_PRIVATE();
        };
    }
}
