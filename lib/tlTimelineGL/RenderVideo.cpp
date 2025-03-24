// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <tlTimeline/RenderUtil.h>

#include <dtk/gl/GL.h>
#include <dtk/gl/Mesh.h>
#include <dtk/gl/Util.h>
#include <dtk/core/Math.h>

namespace tl
{
    namespace timeline_gl
    {
        void Render::drawBackground(
            const std::vector<dtk::Box2I>& boxes,
            const dtk::M44F& m,
            const timeline::BackgroundOptions& options)
        {
            const dtk::Box2I rect(dtk::V2I(0, 0), getRenderSize());
            switch (options.type)
            {
            case timeline::Background::Solid:
                IRender::drawRect(rect, options.solidColor);
                break;
            case timeline::Background::Checkers:
                drawColorMesh(
                    dtk::checkers(
                        rect,
                        options.checkersColor.first,
                        options.checkersColor.second,
                        options.checkersSize),
                    dtk::Color4F(1.F, 1.F, 1.F));
                break;
            case timeline::Background::Gradient:
            {
                dtk::TriMesh2F mesh;
                mesh.v.push_back(dtk::V2F(rect.min.x, rect.min.y));
                mesh.v.push_back(dtk::V2F(rect.max.x, rect.min.y));
                mesh.v.push_back(dtk::V2F(rect.max.x, rect.max.y));
                mesh.v.push_back(dtk::V2F(rect.min.x, rect.max.y));
                mesh.c.push_back(dtk::V4F(
                    options.gradientColor.first.r,
                    options.gradientColor.first.g,
                    options.gradientColor.first.b,
                    options.gradientColor.first.a));
                mesh.c.push_back(dtk::V4F(
                    options.gradientColor.second.r,
                    options.gradientColor.second.g,
                    options.gradientColor.second.b,
                    options.gradientColor.second.a));
                mesh.triangles.push_back({
                    dtk::Vertex2(1, 0, 1),
                    dtk::Vertex2(2, 0, 1),
                    dtk::Vertex2(3, 0, 2), });
                mesh.triangles.push_back({
                    dtk::Vertex2(3, 0, 2),
                    dtk::Vertex2(4, 0, 2),
                    dtk::Vertex2(1, 0, 1), });
                drawColorMesh(
                    mesh,
                    dtk::Color4F(1.F, 1.F, 1.F));
                break;
            }
            default: break;
            }

            if (options.outline.enabled && !boxes.empty())
            {
                dtk::Box2I bounds = boxes.front();
                for (size_t i = 1; i < boxes.size(); ++i)
                {
                    bounds.min.x = std::min(bounds.min.x, boxes[i].min.x);
                    bounds.min.y = std::min(bounds.min.y, boxes[i].min.y);
                    bounds.max.x = std::max(bounds.max.x, boxes[i].max.x);
                    bounds.max.y = std::max(bounds.max.y, boxes[i].max.y);
                }

                dtk::TriMesh2F mesh;
                mesh.v.push_back(dtk::V2F(bounds.min.x, bounds.min.y));
                mesh.v.push_back(dtk::V2F(bounds.max.x + 1, bounds.min.y));
                mesh.v.push_back(dtk::V2F(bounds.max.x + 1, bounds.max.y + 1));
                mesh.v.push_back(dtk::V2F(bounds.min.x, bounds.max.y + 1));
                for (auto& v : mesh.v)
                {
                    const dtk::V3F v3 = dtk::V3F(v.x, v.y, 0.F) * m;
                    v.x = v3.x;
                    v.y = v3.y;
                }
                mesh.v.push_back(dtk::round(
                    dtk::normalize(mesh.v[0] - mesh.v[1]) * options.outline.width +
                    dtk::normalize(mesh.v[0] - mesh.v[3]) * options.outline.width +
                    mesh.v[0]));
                mesh.v.push_back(dtk::round(
                    dtk::normalize(mesh.v[1] - mesh.v[2]) * options.outline.width +
                    dtk::normalize(mesh.v[1] - mesh.v[0]) * options.outline.width +
                    mesh.v[1]));
                mesh.v.push_back(dtk::round(
                    dtk::normalize(mesh.v[2] - mesh.v[1]) * options.outline.width +
                    dtk::normalize(mesh.v[2] - mesh.v[3]) * options.outline.width +
                    mesh.v[2]));
                mesh.v.push_back(dtk::round(
                    dtk::normalize(mesh.v[3] - mesh.v[0]) * options.outline.width +
                    dtk::normalize(mesh.v[3] - mesh.v[2]) * options.outline.width +
                    mesh.v[3]));
                mesh.triangles.push_back({ dtk::Vertex2(1), dtk::Vertex2(2), dtk::Vertex2(5) });
                mesh.triangles.push_back({ dtk::Vertex2(2), dtk::Vertex2(6), dtk::Vertex2(5) });
                mesh.triangles.push_back({ dtk::Vertex2(2), dtk::Vertex2(3), dtk::Vertex2(6) });
                mesh.triangles.push_back({ dtk::Vertex2(3), dtk::Vertex2(7), dtk::Vertex2(6) });
                mesh.triangles.push_back({ dtk::Vertex2(3), dtk::Vertex2(4), dtk::Vertex2(7) });
                mesh.triangles.push_back({ dtk::Vertex2(4), dtk::Vertex2(8), dtk::Vertex2(7) });
                mesh.triangles.push_back({ dtk::Vertex2(4), dtk::Vertex2(1), dtk::Vertex2(8) });
                mesh.triangles.push_back({ dtk::Vertex2(1), dtk::Vertex2(5), dtk::Vertex2(8) });
                drawMesh(mesh, options.outline.color);
            }
        }

        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<dtk::Box2I>& boxes,
            const std::vector<dtk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            dtk::ImageType colorBuffer)
        {
            switch (compareOptions.compare)
            {
            case timeline::Compare::A:
                _drawVideoA(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions,
                    colorBuffer);
                break;
            case timeline::Compare::B:
                _drawVideoB(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions,
                    colorBuffer);
                break;
            case timeline::Compare::Wipe:
                _drawVideoWipe(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions,
                    colorBuffer);
                break;
            case timeline::Compare::Overlay:
                _drawVideoOverlay(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions,
                    colorBuffer);
                break;
            case timeline::Compare::Difference:
                if (videoData.size() > 1)
                {
                    _drawVideoDifference(
                        videoData,
                        boxes,
                        imageOptions,
                        displayOptions,
                        compareOptions,
                        colorBuffer);
                }
                else
                {
                    _drawVideoA(
                        videoData,
                        boxes,
                        imageOptions,
                        displayOptions,
                        compareOptions,
                        colorBuffer);
                }
                break;
            case timeline::Compare::Horizontal:
            case timeline::Compare::Vertical:
            case timeline::Compare::Tile:
                _drawVideoTile(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions,
                    colorBuffer);
                break;
            default: break;
            }
        }

        void Render::_drawVideoA(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<dtk::Box2I>& boxes,
            const std::vector<dtk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            dtk::ImageType colorBuffer)
        {
            if (!videoData.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoData[0],
                    boxes[0],
                    !imageOptions.empty() ? std::make_shared<dtk::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions(),
                    colorBuffer);
            }
        }

        void Render::_drawVideoB(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<dtk::Box2I>& boxes,
            const std::vector<dtk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            dtk::ImageType colorBuffer)
        {
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1],
                    boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<dtk::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions(),
                    colorBuffer);
            }
        }

        void Render::_drawVideoWipe(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<dtk::Box2I>& boxes,
            const std::vector<dtk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            dtk::ImageType colorBuffer)
        {
            DTK_P();

            float radius = 0.F;
            float x = 0.F;
            float y = 0.F;
            if (!boxes.empty())
            {
                radius = std::max(boxes[0].w(), boxes[0].h()) * 2.5F;
                x = boxes[0].w() * compareOptions.wipeCenter.x;
                y = boxes[0].h() * compareOptions.wipeCenter.y;
            }
            const float rotation = compareOptions.wipeRotation;
            dtk::V2F pts[4];
            for (size_t i = 0; i < 4; ++i)
            {
                float rad = dtk::deg2rad(rotation + 90.F * i + 90.F);
                pts[i].x = cos(rad) * radius + x;
                pts[i].y = sin(rad) * radius + y;
            }

            dtk::gl::SetAndRestore stencilTest(GL_STENCIL_TEST, GL_TRUE);

            const dtk::Size2I renderSize = getRenderSize();
            const dtk::Box2I viewport = getViewport();
            glViewport(
                viewport.x(),
                renderSize.h - viewport.h() - viewport.y(),
                viewport.w(),
                viewport.h());
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind();
            p.shaders["wipe"]->setUniform("color", dtk::Color4F(1.F, 0.F, 0.F));
            {
                if (p.vbos["wipe"])
                {
                    dtk::TriMesh2F mesh;
                    mesh.v.push_back(pts[0]);
                    mesh.v.push_back(pts[1]);
                    mesh.v.push_back(pts[2]);
                    dtk::Triangle2 tri;
                    tri.v[0] = 1;
                    tri.v[1] = 2;
                    tri.v[2] = 3;
                    mesh.triangles.push_back(tri);
                    p.vbos["wipe"]->copy(convert(mesh, p.vbos["wipe"]->getType()));
                }
                if (p.vaos["wipe"])
                {
                    p.vaos["wipe"]->bind();
                    p.vaos["wipe"]->draw(GL_TRIANGLES, 0, p.vbos["wipe"]->getSize());
                }
            }
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if (!videoData.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoData[0],
                    boxes[0],
                    !imageOptions.empty() ? std::make_shared<dtk::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions(),
                    colorBuffer);
            }

            glViewport(
                viewport.x(),
                renderSize.h - viewport.h() - viewport.y(),
                viewport.w(),
                viewport.h());
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind();
            p.shaders["wipe"]->setUniform("color", dtk::Color4F(0.F, 1.F, 0.F));
            {
                if (p.vbos["wipe"])
                {
                    dtk::TriMesh2F mesh;
                    mesh.v.push_back(pts[2]);
                    mesh.v.push_back(pts[3]);
                    mesh.v.push_back(pts[0]);
                    dtk::Triangle2 tri;
                    tri.v[0] = 1;
                    tri.v[1] = 2;
                    tri.v[2] = 3;
                    mesh.triangles.push_back(tri);
                    p.vbos["wipe"]->copy(convert(mesh, p.vbos["wipe"]->getType()));
                }
                if (p.vaos["wipe"])
                {
                    p.vaos["wipe"]->bind();
                    p.vaos["wipe"]->draw(GL_TRIANGLES, 0, p.vbos["wipe"]->getSize());
                }
            }
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1],
                    boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<dtk::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions(),
                    colorBuffer);
            }
        }

        void Render::_drawVideoOverlay(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<dtk::Box2I>& boxes,
            const std::vector<dtk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            dtk::ImageType colorBuffer)
        {
            DTK_P();

            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1],
                    boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<dtk::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions(),
                    colorBuffer);
            }
            if (!videoData.empty() && !boxes.empty())
            {
                const dtk::Size2I offscreenBufferSize(
                    boxes[0].w(),
                    boxes[0].h());
                dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.color = colorBuffer;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                if (doCreate(
                    p.buffers["overlay"],
                    offscreenBufferSize,
                    offscreenBufferOptions))
                {
                    p.buffers["overlay"] = dtk::gl::OffscreenBuffer::create(
                        offscreenBufferSize,
                        offscreenBufferOptions);
                }

                if (p.buffers["overlay"])
                {
                    const dtk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                    dtk::gl::OffscreenBufferBinding binding(p.buffers["overlay"]);
                    glViewport(
                        0,
                        0,
                        offscreenBufferSize.w,
                        offscreenBufferSize.h);
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform(
                        "transform.mvp",
                        dtk::ortho(
                            0.F,
                            static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h),
                            0.F,
                            -1.F,
                            1.F));

                    _drawVideo(
                        videoData[0],
                        dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<dtk::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions(),
                        colorBuffer);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform("transform.mvp", getTransform());
                }

                if (p.buffers["overlay"])
                {
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                    const dtk::Size2I renderSize = getRenderSize();
                    const dtk::Box2I viewport = getViewport();
                    glViewport(
                        viewport.x(),
                        renderSize.h - viewport.h() - viewport.y(),
                        viewport.w(),
                        viewport.h());

                    p.shaders["overlay"]->bind();
                    p.shaders["overlay"]->setUniform("color", dtk::Color4F(1.F, 1.F, 1.F, compareOptions.overlay));
                    p.shaders["overlay"]->setUniform("textureSampler", 0);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    glBindTexture(GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(
                            dtk::mesh(boxes[0], true),
                            p.vbos["video"]->getType()));
                    }
                    if (p.vaos["video"])
                    {
                        p.vaos["video"]->bind();
                        p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                    }
                }
            }
        }

        void Render::_drawVideoDifference(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<dtk::Box2I>& boxes,
            const std::vector<dtk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            dtk::ImageType colorBuffer)
        {
            DTK_P();
            if (!videoData.empty() && !boxes.empty())
            {
                const dtk::Size2I offscreenBufferSize(
                    boxes[0].w(),
                    boxes[0].h());
                dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.color = colorBuffer;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                if (doCreate(
                    p.buffers["difference0"],
                    offscreenBufferSize,
                    offscreenBufferOptions))
                {
                    p.buffers["difference0"] = dtk::gl::OffscreenBuffer::create(
                        offscreenBufferSize,
                        offscreenBufferOptions);
                }

                if (p.buffers["difference0"])
                {
                    const dtk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                    dtk::gl::OffscreenBufferBinding binding(p.buffers["difference0"]);
                    glViewport(
                        0,
                        0,
                        offscreenBufferSize.w,
                        offscreenBufferSize.h);
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform(
                        "transform.mvp",
                        dtk::ortho(
                            0.F,
                            static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h),
                            0.F,
                            -1.F,
                            1.F));

                    _drawVideo(
                        videoData[0],
                        dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<dtk::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions(),
                        colorBuffer);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform("transform.mvp", getTransform());
                }

                if (videoData.size() > 1)
                {
                    offscreenBufferOptions = dtk::gl::OffscreenBufferOptions();
                    offscreenBufferOptions.color = colorBuffer;
                    if (displayOptions.size() > 1)
                    {
                        offscreenBufferOptions.colorFilters = displayOptions[1].imageFilters;
                    }
                    if (doCreate(
                        p.buffers["difference1"],
                        offscreenBufferSize,
                        offscreenBufferOptions))
                    {
                        p.buffers["difference1"] = dtk::gl::OffscreenBuffer::create(
                            offscreenBufferSize,
                            offscreenBufferOptions);
                    }

                    if (p.buffers["difference1"])
                    {
                        const dtk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                        dtk::gl::OffscreenBufferBinding binding(p.buffers["difference1"]);
                        glViewport(
                            0,
                            0,
                            offscreenBufferSize.w,
                            offscreenBufferSize.h);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);

                        p.shaders["display"]->bind();
                        p.shaders["display"]->setUniform(
                            "transform.mvp",
                            dtk::ortho(
                                0.F,
                                static_cast<float>(offscreenBufferSize.w),
                                static_cast<float>(offscreenBufferSize.h),
                                0.F,
                                -1.F,
                                1.F));

                        _drawVideo(
                            videoData[1],
                            dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                            imageOptions.size() > 1 ? std::make_shared<dtk::ImageOptions>(imageOptions[1]) : nullptr,
                            displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions(),
                            colorBuffer);
                    }
                }
                else
                {
                    p.buffers["difference1"].reset();
                }

                if (p.buffers["difference0"] && p.buffers["difference1"])
                {
                    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                    const dtk::Size2I renderSize = getRenderSize();
                    const dtk::Box2I viewport = getViewport();
                    glViewport(
                        viewport.x(),
                        renderSize.h - viewport.h() - viewport.y(),
                        viewport.w(),
                        viewport.h());

                    p.shaders["difference"]->bind();
                    p.shaders["difference"]->setUniform("textureSampler", 0);
                    p.shaders["difference"]->setUniform("textureSamplerB", 1);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    glBindTexture(GL_TEXTURE_2D, p.buffers["difference0"]->getColorID());

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                    glBindTexture(GL_TEXTURE_2D, p.buffers["difference1"]->getColorID());

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(
                            dtk::mesh(boxes[0], true),
                            p.vbos["video"]->getType()));
                    }
                    if (p.vaos["video"])
                    {
                        p.vaos["video"]->bind();
                        p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                    }
                }
            }
        }

        void Render::_drawVideoTile(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<dtk::Box2I>& boxes,
            const std::vector<dtk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            dtk::ImageType colorBuffer)
        {
            for (size_t i = 0; i < videoData.size() && i < boxes.size(); ++i)
            {
                _drawVideo(
                    videoData[i],
                    boxes[i],
                    i < imageOptions.size() ? std::make_shared<dtk::ImageOptions>(imageOptions[i]) : nullptr,
                    i < displayOptions.size() ? displayOptions[i] : timeline::DisplayOptions(),
                    colorBuffer);
            }
        }
        
        namespace
        {
            float knee(float x, float f)
            {
                return logf(x * f + 1.F) / f;
            }

            float knee2(float x, float y)
            {
                float f0 = 0.F;
                float f1 = 1.F;
                while (knee(x, f1) > y)
                {
                    f0 = f1;
                    f1 = f1 * 2.F;
                }
                for (size_t i = 0; i < 30; ++i)
                {
                    const float f2 = (f0 + f1) / 2.F;
                    if (knee(x, f2) < y)
                    {
                        f1 = f2;
                    }
                    else
                    {
                        f0 = f2;
                    }
                }
                return (f0 + f1) / 2.F;
            }
        }

        void Render::_drawVideo(
            const timeline::VideoData& videoData,
            const dtk::Box2I& box,
            const std::shared_ptr<dtk::ImageOptions>& imageOptions,
            const timeline::DisplayOptions& displayOptions,
            dtk::ImageType colorBuffer)
        {
            DTK_P();
            
            GLint viewportPrev[4] = { 0, 0, 0, 0 };
            glGetIntegerv(GL_VIEWPORT, viewportPrev);

            auto imageShader = p.baseRender->getShader("image");
            imageShader->bind();
            const auto transform = dtk::ortho(
                0.F,
                static_cast<float>(box.w()),
                static_cast<float>(box.h()),
                0.F,
                -1.F,
                1.F);
            imageShader->setUniform("transform.mvp", transform);

            const dtk::Size2I& offscreenBufferSize = box.size();
            dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.color = colorBuffer;
            offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            if (doCreate(
                p.buffers["video"],
                offscreenBufferSize,
                offscreenBufferOptions))
            {
                p.buffers["video"] = dtk::gl::OffscreenBuffer::create(
                    offscreenBufferSize,
                    offscreenBufferOptions);
            }

            if (p.buffers["video"])
            {
                const dtk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                dtk::gl::OffscreenBufferBinding binding(p.buffers["video"]);
                glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                for (const auto& layer : videoData.layers)
                {
                    switch (layer.transition)
                    {
                        case timeline::Transition::Dissolve:
                        {
                            if (layer.image && layer.imageB)
                            {
                                if (doCreate(
                                    p.buffers["dissolve"],
                                    offscreenBufferSize,
                                    offscreenBufferOptions))
                                {
                                    p.buffers["dissolve"] = dtk::gl::OffscreenBuffer::create(
                                        offscreenBufferSize,
                                        offscreenBufferOptions);
                                }
                                if (doCreate(
                                    p.buffers["dissolve2"],
                                    offscreenBufferSize,
                                    offscreenBufferOptions))
                                {
                                    p.buffers["dissolve2"] = dtk::gl::OffscreenBuffer::create(
                                        offscreenBufferSize,
                                        offscreenBufferOptions);
                                }
                                if (p.buffers["dissolve"])
                                {
                                    dtk::gl::OffscreenBufferBinding binding(p.buffers["dissolve"]);
                                    glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                                    glClearColor(0.F, 0.F, 0.F, 0.F);
                                    glClear(GL_COLOR_BUFFER_BIT);
                                    auto dissolveImageOptions = imageOptions.get() ? *imageOptions : layer.imageOptions;
                                    dissolveImageOptions.alphaBlend = dtk::AlphaBlend::None;
                                    IRender::drawImage(
                                        layer.image,
                                        timeline::getBox(
                                            layer.image->getAspect(),
                                            dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                        dtk::Color4F(1.F, 1.F, 1.F),
                                        dissolveImageOptions);
                                }
                                if (p.buffers["dissolve2"])
                                {
                                    dtk::gl::OffscreenBufferBinding binding(p.buffers["dissolve2"]);
                                    glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                                    glClearColor(0.F, 0.F, 0.F, 0.F);
                                    glClear(GL_COLOR_BUFFER_BIT);
                                    auto dissolveImageOptions = imageOptions.get() ? *imageOptions : layer.imageOptionsB;
                                    dissolveImageOptions.alphaBlend = dtk::AlphaBlend::None;
                                    IRender::drawImage(
                                        layer.imageB,
                                        timeline::getBox(
                                            layer.imageB->getAspect(),
                                            dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                        dtk::Color4F(1.F, 1.F, 1.F),
                                        dissolveImageOptions);
                                }
                                if (p.buffers["dissolve"] && p.buffers["dissolve2"])
                                {
                                    p.shaders["dissolve"]->bind();
                                    p.shaders["dissolve"]->setUniform("transform.mvp", transform);
                                    p.shaders["dissolve"]->setUniform("dissolve", layer.transitionValue);
                                    p.shaders["dissolve"]->setUniform("textureSampler", 0);
                                    p.shaders["dissolve"]->setUniform("textureSampler2", 1);

                                    dtk::gl::setAlphaBlend(dtk::AlphaBlend::Straight);

                                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                                    glBindTexture(GL_TEXTURE_2D, p.buffers["dissolve"]->getColorID());
                                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                                    glBindTexture(GL_TEXTURE_2D, p.buffers["dissolve2"]->getColorID());
                                    if (p.vbos["video"])
                                    {
                                        p.vbos["video"]->copy(convert(
                                            dtk::mesh(dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h), true),
                                            p.vbos["video"]->getType()));
                                    }
                                    if (p.vaos["video"])
                                    {
                                        p.vaos["video"]->bind();
                                        p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                                    }
                                }
                            }
                            else if (layer.image)
                            {
                                IRender::drawImage(
                                    layer.image,
                                    timeline::getBox(
                                        layer.image->getAspect(),
                                        dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                    dtk::Color4F(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptions);
                            }
                            else if (layer.imageB)
                            {
                                IRender::drawImage(
                                    layer.imageB,
                                    timeline::getBox(
                                        layer.imageB->getAspect(),
                                        dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                    dtk::Color4F(1.F, 1.F, 1.F, layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptionsB);
                            }
                            break;
                        }
                    default:
                        if (layer.image)
                        {
                            IRender::drawImage(
                                layer.image,
                                timeline::getBox(
                                    layer.image->getAspect(),
                                    dtk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                dtk::Color4F(1.F, 1.F, 1.F),
                                imageOptions.get() ? *imageOptions : layer.imageOptions);
                        }
                        break;
                    }
                }
            }

            if (p.buffers["video"])
            {
                glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

                glViewport(
                    viewportPrev[0],
                    viewportPrev[1],
                    viewportPrev[2],
                    viewportPrev[3]);

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform("textureSampler", 0);
                p.shaders["display"]->setUniform("channels", static_cast<int>(displayOptions.channels));
                p.shaders["display"]->setUniform("mirrorX", displayOptions.mirror.x);
                p.shaders["display"]->setUniform("mirrorY", displayOptions.mirror.y);
                const bool colorMatrixEnabled =
                    displayOptions.color != timeline::Color() &&
                    displayOptions.color.enabled;
                p.shaders["display"]->setUniform("colorEnabled", colorMatrixEnabled);
                p.shaders["display"]->setUniform("colorAdd", displayOptions.color.add);
                if (colorMatrixEnabled)
                {
                    p.shaders["display"]->setUniform("colorMatrix", color(displayOptions.color));
                }
                p.shaders["display"]->setUniform(
                    "colorInvert",
                    displayOptions.color.enabled ? displayOptions.color.invert : false);
                p.shaders["display"]->setUniform("levelsEnabled", displayOptions.levels.enabled);
                p.shaders["display"]->setUniform("levels.inLow", displayOptions.levels.inLow);
                p.shaders["display"]->setUniform("levels.inHigh", displayOptions.levels.inHigh);
                p.shaders["display"]->setUniform(
                    "levels.gamma",
                    displayOptions.levels.gamma > 0.F ? (1.F / displayOptions.levels.gamma) : 1000000.F);
                p.shaders["display"]->setUniform("levels.outLow", displayOptions.levels.outLow);
                p.shaders["display"]->setUniform("levels.outHigh", displayOptions.levels.outHigh);
                p.shaders["display"]->setUniform("exrDisplayEnabled", displayOptions.exrDisplay.enabled);
                if (displayOptions.exrDisplay.enabled)
                {
                    const float v = powf(2.F, displayOptions.exrDisplay.exposure + 2.47393F);
                    const float d = displayOptions.exrDisplay.defog;
                    const float k = powf(2.F, displayOptions.exrDisplay.kneeLow);
                    const float f = knee2(
                        powf(2.F, displayOptions.exrDisplay.kneeHigh) - k,
                        powf(2.F, 3.5F) - k);
                    p.shaders["display"]->setUniform("exrDisplay.v", v);
                    p.shaders["display"]->setUniform("exrDisplay.d", d);
                    p.shaders["display"]->setUniform("exrDisplay.k", k);
                    p.shaders["display"]->setUniform("exrDisplay.f", f);
                    const float gamma =
                        displayOptions.levels.gamma > 0.F ?
                        (1.F / displayOptions.levels.gamma) :
                        1000000.F;
                    p.shaders["display"]->setUniform("exrDisplay.g", gamma );
                }
                p.shaders["display"]->setUniform(
                    "softClip",
                    displayOptions.softClip.enabled ? displayOptions.softClip.value : 0.F);
                p.shaders["display"]->setUniform(
                    "videoLevels",
                    static_cast<int>(displayOptions.videoLevels));

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                glBindTexture(GL_TEXTURE_2D, p.buffers["video"]->getColorID());
                size_t texturesOffset = 1;
#if defined(TLRENDER_OCIO)
                if (p.ocioData)
                {
                    for (size_t i = 0; i < p.ocioData->textures.size(); ++i)
                    {
                        glActiveTexture(GL_TEXTURE0 + texturesOffset + i);
                        glBindTexture(
                            p.ocioData->textures[i].type,
                            p.ocioData->textures[i].id);
                    }
                    texturesOffset += p.ocioData->textures.size();
                }
                if (p.lutData)
                {
                    for (size_t i = 0; i < p.lutData->textures.size(); ++i)
                    {
                        glActiveTexture(GL_TEXTURE0 + texturesOffset + i);
                        glBindTexture(
                            p.lutData->textures[i].type,
                            p.lutData->textures[i].id);
                    }
                    texturesOffset += p.lutData->textures.size();
                }
#endif // TLRENDER_OCIO

                if (p.vbos["video"])
                {
                    p.vbos["video"]->copy(convert(
                        dtk::mesh(box, true),
                        p.vbos["video"]->getType()));
                }
                if (p.vaos["video"])
                {
                    p.vaos["video"]->bind();
                    p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                }
            }

            imageShader->bind();
            imageShader->setUniform("transform.mvp", getTransform());
        }

        void Render::drawForeground(
            const std::vector<dtk::Box2I>& boxes,
            const dtk::M44F& m,
            const timeline::ForegroundOptions& options)
        {
            if (options.grid.enabled && !boxes.empty())
            {
                const dtk::V3F v0 = dtk::V3F(0.F, 0.F, 0.F) * m;
                const dtk::V3F v1 = dtk::V3F(options.grid.size, 0.F, 0.F) * m;
                if (dtk::length(v1 - v0) > options.grid.lineWidth + 10.F)
                {
                    dtk::Box2I bounds = boxes.front();
                    for (size_t i = 1; i < boxes.size(); ++i)
                    {
                        bounds.min.x = std::min(bounds.min.x, boxes[i].min.x);
                        bounds.min.y = std::min(bounds.min.y, boxes[i].min.y);
                        bounds.max.x = std::max(bounds.max.x, boxes[i].max.x);
                        bounds.max.y = std::max(bounds.max.y, boxes[i].max.y);
                    }

                    const dtk::Size2I& renderSize = getRenderSize();
                    const dtk::Box2F vp(0, 0, renderSize.w, renderSize.h);
                    std::vector<std::pair<dtk::V2F, dtk::V2F> > lines;
                    dtk::LineOptions lineOptions;
                    lineOptions.width = options.grid.lineWidth;
                    for (int y = bounds.min.y; y <= bounds.max.y + 1; y += options.grid.size)
                    {
                        const dtk::V3F v0 = dtk::V3F(bounds.min.x, y, 0.F) * m;
                        const dtk::V3F v1 = dtk::V3F(bounds.max.x + 1, y, 0.F) * m;
                        const dtk::V2F v2(std::round(v0.x), std::round(v0.y));
                        const dtk::V2F v3(std::round(v1.x), std::round(v1.y));
                        if (dtk::intersects(dtk::Box2F(v2, v3), vp))
                        {
                            lines.push_back(std::make_pair(v2, v3));
                        }
                    }
                    for (int x = bounds.min.x; x <= bounds.max.x + 1; x += options.grid.size)
                    {
                        const dtk::V3F v0 = dtk::V3F(x, bounds.min.y, 0.F) * m;
                        const dtk::V3F v1 = dtk::V3F(x, bounds.max.y + 1, 0.F) * m;
                        const dtk::V2F v2(std::round(v0.x), std::round(v0.y));
                        const dtk::V2F v3(std::round(v1.x), std::round(v1.y));
                        if (dtk::intersects(dtk::Box2F(v2, v3), vp))
                        {
                            lines.push_back(std::make_pair(v2, v3));
                        }
                    }
                    drawLines(lines, options.grid.color, lineOptions);
                }
            }
        }
    }
}
