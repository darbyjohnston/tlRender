// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <dtk/gl/GL.h>

namespace tl
{
    namespace timeline_gl
    {
        void Render::drawTexture(
            unsigned int id,
            const dtk::Box2I& box,
            const dtk::Color4F& color)
        {
            TLRENDER_P();
            p.shaders["texture"]->bind();
            p.shaders["texture"]->setUniform("color", color);
            p.shaders["texture"]->setUniform("textureSampler", 0);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
            glBindTexture(GL_TEXTURE_2D, id);

            if (p.vbos["texture"])
            {
                p.vbos["texture"]->copy(convert(dtk::mesh(box), p.vbos["texture"]->getType()));
            }
            if (p.vaos["texture"])
            {
                p.vaos["texture"]->bind();
                p.vaos["texture"]->draw(GL_TRIANGLES, 0, p.vbos["texture"]->getSize());
            }
        }

        void Render::drawRect(
            const dtk::Box2F& rect,
            const dtk::Color4F& color)
        {
            _p->baseRender->drawRect(rect, color);
        }

        void Render::drawRects(
            const std::vector<dtk::Box2F>& rects,
            const dtk::Color4F& color)
        {
            _p->baseRender->drawRects(rects, color);
        }

        void Render::drawLine(
            const dtk::V2F& v0,
            const dtk::V2F& v1,
            const dtk::Color4F& color,
            const dtk::LineOptions& options)
        {
            _p->baseRender->drawLine(v0, v1, color, options);
        }

        void Render::drawLines(
            const std::vector<std::pair<dtk::V2F, dtk::V2F> >& v,
            const dtk::Color4F& color,
            const dtk::LineOptions& options)
        {
            _p->baseRender->drawLines(v, color, options);
        }

        void Render::drawMesh(
            const dtk::TriMesh2F& mesh,
            const dtk::Color4F& color,
            const dtk::V2F& pos)
        {
            _p->baseRender->drawMesh(mesh, color, pos);
        }

        void Render::drawColorMesh(
            const dtk::TriMesh2F& mesh,
            const dtk::Color4F& color,
            const dtk::V2F& pos)
        {
            _p->baseRender->drawColorMesh(mesh, color, pos);
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<dtk::Glyph> >& glyphs,
            const dtk::FontMetrics& fontMetrics,
            const dtk::V2F& pos,
            const dtk::Color4F& color)
        {
            _p->baseRender->drawText(glyphs, fontMetrics, pos, color);
        }

        void Render::drawImage(
            const std::shared_ptr<dtk::Image>& image,
            const dtk::TriMesh2F& mesh,
            const dtk::Color4F& color,
            const dtk::ImageOptions& options)
        {
            _p->baseRender->drawImage(image, mesh, color, options);
        }

        void Render::drawImage(
            const std::shared_ptr<dtk::Image>& image,
            const dtk::Box2F& rect,
            const dtk::Color4F& color,
            const dtk::ImageOptions& options)
        {
            _p->baseRender->drawImage(image, rect, color, options);
        }
    }
}
