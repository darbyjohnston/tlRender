// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <feather-tk/gl/Render.h>
#include <feather-tk/core/LRUCache.h>

namespace tl
{
    //! Timeline OpenGL support
    namespace timeline_gl
    {
        //! OpenGL renderer.
        class Render : public timeline::IRender
        {
            FTK_NON_COPYABLE(Render);

        protected:
            void _init(
                const std::shared_ptr<ftk::LogSystem>&,
                const std::shared_ptr<ftk::gl::TextureCache>&);

            Render();

        public:
            virtual ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                const std::shared_ptr<ftk::LogSystem>& = nullptr,
                const std::shared_ptr<ftk::gl::TextureCache>& = nullptr);

            const std::shared_ptr<ftk::gl::TextureCache>& getTextureCache() const;

            void setOCIOOptions(const timeline::OCIOOptions&) override;
            void setLUTOptions(const timeline::LUTOptions&) override;

            void drawTexture(
                unsigned int,
                const ftk::Box2I&,
                const ftk::Color4F & = ftk::Color4F(1.F, 1.F, 1.F),
                ftk::AlphaBlend = ftk::AlphaBlend::Straight) override;
            void drawBackground(
                const std::vector<ftk::Box2I>&,
                const ftk::M44F&,
                const timeline::BackgroundOptions&) override;
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions(),
                ftk::ImageType colorBuffer = ftk::ImageType::RGBA_U8) override;
            void drawForeground(
                const std::vector<ftk::Box2I>&,
                const ftk::M44F&,
                const timeline::ForegroundOptions&) override;

            void begin(
                const ftk::Size2I&,
                const ftk::RenderOptions& = ftk::RenderOptions()) override;
            void end() override;
            ftk::Size2I getRenderSize() const override;
            void setRenderSize(const ftk::Size2I&) override;
            ftk::RenderOptions getRenderOptions() const override;
            ftk::Box2I getViewport() const override;
            void setViewport(const ftk::Box2I&) override;
            void clearViewport(const ftk::Color4F&) override;
            bool getClipRectEnabled() const override;
            void setClipRectEnabled(bool) override;
            ftk::Box2I getClipRect() const override;
            void setClipRect(const ftk::Box2I&) override;
            ftk::M44F getTransform() const override;
            void setTransform(const ftk::M44F&) override;
            void drawRect(
                const ftk::Box2F&,
                const ftk::Color4F&) override;
            void drawRects(
                const std::vector<ftk::Box2F>&,
                const ftk::Color4F&) override;
            void drawLine(
                const ftk::V2F&,
                const ftk::V2F&,
                const ftk::Color4F&,
                const ftk::LineOptions& = ftk::LineOptions()) override;
            void drawLines(
                const std::vector<std::pair<ftk::V2F, ftk::V2F> >&,
                const ftk::Color4F&,
                const ftk::LineOptions& = ftk::LineOptions()) override;
            void drawMesh(
                const ftk::TriMesh2F&,
                const ftk::Color4F& = ftk::Color4F(1.F, 1.F, 1.F, 1.F),
                const ftk::V2F& pos = ftk::V2F()) override;
            void drawColorMesh(
                const ftk::TriMesh2F&,
                const ftk::Color4F& = ftk::Color4F(1.F, 1.F, 1.F, 1.F),
                const ftk::V2F& pos = ftk::V2F()) override;
            void drawText(
                const std::vector<std::shared_ptr<ftk::Glyph> >&,
                const ftk::FontMetrics&,
                const ftk::V2F& position,
                const ftk::Color4F& = ftk::Color4F(1.F, 1.F, 1.F, 1.F)) override;
            void drawImage(
                const std::shared_ptr<ftk::Image>&,
                const ftk::TriMesh2F&,
                const ftk::Color4F& = ftk::Color4F(1.F, 1.F, 1.F, 1.F),
                const ftk::ImageOptions& = ftk::ImageOptions()) override;
            void drawImage(
                const std::shared_ptr<ftk::Image>&,
                const ftk::Box2F&,
                const ftk::Color4F& = ftk::Color4F(1.F, 1.F, 1.F, 1.F),
                const ftk::ImageOptions& = ftk::ImageOptions()) override;

        private:
            void _displayShader();

            void _drawVideoA(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                ftk::ImageType colorBuffer);
            void _drawVideoB(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                ftk::ImageType colorBuffer);
            void _drawVideoWipe(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                ftk::ImageType colorBuffer);
            void _drawVideoOverlay(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                ftk::ImageType colorBuffer);
            void _drawVideoDifference(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                ftk::ImageType colorBuffer);
            void _drawVideoTile(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                ftk::ImageType colorBuffer);
            void _drawVideo(
                const timeline::VideoData&,
                const ftk::Box2I&,
                const std::shared_ptr<ftk::ImageOptions>&,
                const timeline::DisplayOptions&,
                ftk::ImageType colorBuffer);

            FTK_PRIVATE();
        };
    }
}
