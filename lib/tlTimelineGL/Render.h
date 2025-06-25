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
            FEATHER_TK_NON_COPYABLE(Render);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::gl::TextureCache>&);

            Render();

        public:
            virtual ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::gl::TextureCache>& = nullptr);

            const std::shared_ptr<feather_tk::gl::TextureCache>& getTextureCache() const;

            void setOCIOOptions(const timeline::OCIOOptions&) override;
            void setLUTOptions(const timeline::LUTOptions&) override;

            void drawTexture(
                unsigned int,
                const feather_tk::Box2I&,
                const feather_tk::Color4F & = feather_tk::Color4F(1.F, 1.F, 1.F),
                feather_tk::AlphaBlend = feather_tk::AlphaBlend::Straight) override;
            void drawBackground(
                const std::vector<feather_tk::Box2I>&,
                const feather_tk::M44F&,
                const timeline::BackgroundOptions&) override;
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<feather_tk::Box2I>&,
                const std::vector<feather_tk::ImageOptions>& = {},
                const std::vector<timeline::DisplayOptions>& = {},
                const timeline::CompareOptions& = timeline::CompareOptions(),
                feather_tk::ImageType colorBuffer = feather_tk::ImageType::RGBA_U8) override;
            void drawForeground(
                const std::vector<feather_tk::Box2I>&,
                const feather_tk::M44F&,
                const timeline::ForegroundOptions&) override;

            void begin(
                const feather_tk::Size2I&,
                const feather_tk::RenderOptions& = feather_tk::RenderOptions()) override;
            void end() override;
            feather_tk::Size2I getRenderSize() const override;
            void setRenderSize(const feather_tk::Size2I&) override;
            feather_tk::RenderOptions getRenderOptions() const override;
            feather_tk::Box2I getViewport() const override;
            void setViewport(const feather_tk::Box2I&) override;
            void clearViewport(const feather_tk::Color4F&) override;
            bool getClipRectEnabled() const override;
            void setClipRectEnabled(bool) override;
            feather_tk::Box2I getClipRect() const override;
            void setClipRect(const feather_tk::Box2I&) override;
            feather_tk::M44F getTransform() const override;
            void setTransform(const feather_tk::M44F&) override;
            void drawRect(
                const feather_tk::Box2F&,
                const feather_tk::Color4F&) override;
            void drawRects(
                const std::vector<feather_tk::Box2F>&,
                const feather_tk::Color4F&) override;
            void drawLine(
                const feather_tk::V2F&,
                const feather_tk::V2F&,
                const feather_tk::Color4F&,
                const feather_tk::LineOptions& = feather_tk::LineOptions()) override;
            void drawLines(
                const std::vector<std::pair<feather_tk::V2F, feather_tk::V2F> >&,
                const feather_tk::Color4F&,
                const feather_tk::LineOptions& = feather_tk::LineOptions()) override;
            void drawMesh(
                const feather_tk::TriMesh2F&,
                const feather_tk::Color4F& = feather_tk::Color4F(1.F, 1.F, 1.F, 1.F),
                const feather_tk::V2F& pos = feather_tk::V2F()) override;
            void drawColorMesh(
                const feather_tk::TriMesh2F&,
                const feather_tk::Color4F& = feather_tk::Color4F(1.F, 1.F, 1.F, 1.F),
                const feather_tk::V2F& pos = feather_tk::V2F()) override;
            void drawText(
                const std::vector<std::shared_ptr<feather_tk::Glyph> >&,
                const feather_tk::FontMetrics&,
                const feather_tk::V2F& position,
                const feather_tk::Color4F& = feather_tk::Color4F(1.F, 1.F, 1.F, 1.F)) override;
            void drawImage(
                const std::shared_ptr<feather_tk::Image>&,
                const feather_tk::TriMesh2F&,
                const feather_tk::Color4F& = feather_tk::Color4F(1.F, 1.F, 1.F, 1.F),
                const feather_tk::ImageOptions& = feather_tk::ImageOptions()) override;
            void drawImage(
                const std::shared_ptr<feather_tk::Image>&,
                const feather_tk::Box2F&,
                const feather_tk::Color4F& = feather_tk::Color4F(1.F, 1.F, 1.F, 1.F),
                const feather_tk::ImageOptions& = feather_tk::ImageOptions()) override;

        private:
            void _displayShader();

            void _drawVideoA(
                const std::vector<timeline::VideoData>&,
                const std::vector<feather_tk::Box2I>&,
                const std::vector<feather_tk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                feather_tk::ImageType colorBuffer);
            void _drawVideoB(
                const std::vector<timeline::VideoData>&,
                const std::vector<feather_tk::Box2I>&,
                const std::vector<feather_tk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                feather_tk::ImageType colorBuffer);
            void _drawVideoWipe(
                const std::vector<timeline::VideoData>&,
                const std::vector<feather_tk::Box2I>&,
                const std::vector<feather_tk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                feather_tk::ImageType colorBuffer);
            void _drawVideoOverlay(
                const std::vector<timeline::VideoData>&,
                const std::vector<feather_tk::Box2I>&,
                const std::vector<feather_tk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                feather_tk::ImageType colorBuffer);
            void _drawVideoDifference(
                const std::vector<timeline::VideoData>&,
                const std::vector<feather_tk::Box2I>&,
                const std::vector<feather_tk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                feather_tk::ImageType colorBuffer);
            void _drawVideoTile(
                const std::vector<timeline::VideoData>&,
                const std::vector<feather_tk::Box2I>&,
                const std::vector<feather_tk::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                feather_tk::ImageType colorBuffer);
            void _drawVideo(
                const timeline::VideoData&,
                const feather_tk::Box2I&,
                const std::shared_ptr<feather_tk::ImageOptions>&,
                const timeline::DisplayOptions&,
                feather_tk::ImageType colorBuffer);

            FEATHER_TK_PRIVATE();
        };
    }
}
