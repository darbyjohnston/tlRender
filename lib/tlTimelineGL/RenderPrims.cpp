// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <ftk/GL/GL.h>

namespace tl
{
    namespace timeline_gl
    {
        void Render::drawRect(
            const ftk::Box2F& rect,
            const ftk::Color4F& color)
        {
            _p->baseRender->drawRect(rect, color);
        }

        void Render::drawRects(
            const std::vector<ftk::Box2F>& rects,
            const ftk::Color4F& color)
        {
            _p->baseRender->drawRects(rects, color);
        }

        void Render::drawLine(
            const ftk::V2F& v0,
            const ftk::V2F& v1,
            const ftk::Color4F& color,
            const ftk::LineOptions& options)
        {
            _p->baseRender->drawLine(v0, v1, color, options);
        }

        void Render::drawLines(
            const std::vector<std::pair<ftk::V2F, ftk::V2F> >& v,
            const ftk::Color4F& color,
            const ftk::LineOptions& options)
        {
            _p->baseRender->drawLines(v, color, options);
        }

        void Render::drawMesh(
            const ftk::TriMesh2F& mesh,
            const ftk::Color4F& color,
            const ftk::V2F& pos)
        {
            _p->baseRender->drawMesh(mesh, color, pos);
        }

        void Render::drawColorMesh(
            const ftk::TriMesh2F& mesh,
            const ftk::Color4F& color,
            const ftk::V2F& pos)
        {
            _p->baseRender->drawColorMesh(mesh, color, pos);
        }

        void Render::drawTexture(
            unsigned int id,
            const ftk::Box2I& rect,
            bool flipV,
            const ftk::Color4F& color,
            ftk::AlphaBlend alphaBlend)
        {
            _p->baseRender->drawTexture(id, rect, flipV, color, alphaBlend);
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<ftk::Glyph> >& glyphs,
            const ftk::FontMetrics& fontMetrics,
            const ftk::V2F& pos,
            const ftk::Color4F& color)
        {
            _p->baseRender->drawText(glyphs, fontMetrics, pos, color);
        }

        void Render::drawImage(
            const std::shared_ptr<ftk::Image>& image,
            const ftk::TriMesh2F& mesh,
            const ftk::Color4F& color,
            const ftk::ImageOptions& options)
        {
            _p->baseRender->drawImage(image, mesh, color, options);
        }

        void Render::drawImage(
            const std::shared_ptr<ftk::Image>& image,
            const ftk::Box2F& rect,
            const ftk::Color4F& color,
            const ftk::ImageOptions& options)
        {
            _p->baseRender->drawImage(image, rect, color, options);
        }
    }
}
