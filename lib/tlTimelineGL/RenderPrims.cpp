// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <feather-tk/gl/GL.h>

namespace tl
{
    namespace timeline_gl
    {
        void Render::drawRect(
            const feather_tk::Box2F& rect,
            const feather_tk::Color4F& color)
        {
            _p->baseRender->drawRect(rect, color);
        }

        void Render::drawRects(
            const std::vector<feather_tk::Box2F>& rects,
            const feather_tk::Color4F& color)
        {
            _p->baseRender->drawRects(rects, color);
        }

        void Render::drawLine(
            const feather_tk::V2F& v0,
            const feather_tk::V2F& v1,
            const feather_tk::Color4F& color,
            const feather_tk::LineOptions& options)
        {
            _p->baseRender->drawLine(v0, v1, color, options);
        }

        void Render::drawLines(
            const std::vector<std::pair<feather_tk::V2F, feather_tk::V2F> >& v,
            const feather_tk::Color4F& color,
            const feather_tk::LineOptions& options)
        {
            _p->baseRender->drawLines(v, color, options);
        }

        void Render::drawMesh(
            const feather_tk::TriMesh2F& mesh,
            const feather_tk::Color4F& color,
            const feather_tk::V2F& pos)
        {
            _p->baseRender->drawMesh(mesh, color, pos);
        }

        void Render::drawColorMesh(
            const feather_tk::TriMesh2F& mesh,
            const feather_tk::Color4F& color,
            const feather_tk::V2F& pos)
        {
            _p->baseRender->drawColorMesh(mesh, color, pos);
        }

        void Render::drawTexture(
            unsigned int id,
            const feather_tk::Box2I& rect,
            const feather_tk::Color4F& color,
            feather_tk::AlphaBlend alphaBlend)
        {
            _p->baseRender->drawTexture(id, rect, color, alphaBlend);
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<feather_tk::Glyph> >& glyphs,
            const feather_tk::FontMetrics& fontMetrics,
            const feather_tk::V2F& pos,
            const feather_tk::Color4F& color)
        {
            _p->baseRender->drawText(glyphs, fontMetrics, pos, color);
        }

        void Render::drawImage(
            const std::shared_ptr<feather_tk::Image>& image,
            const feather_tk::TriMesh2F& mesh,
            const feather_tk::Color4F& color,
            const feather_tk::ImageOptions& options)
        {
            _p->baseRender->drawImage(image, mesh, color, options);
        }

        void Render::drawImage(
            const std::shared_ptr<feather_tk::Image>& image,
            const feather_tk::Box2F& rect,
            const feather_tk::Color4F& color,
            const feather_tk::ImageOptions& options)
        {
            _p->baseRender->drawImage(image, rect, color, options);
        }
    }
}
