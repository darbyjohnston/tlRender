// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimelineGL/RenderPrivate.h>

#include <tlTimeline/RenderUtil.h>

#include <ftk/GL/GL.h>
#include <ftk/GL/Mesh.h>
#include <ftk/GL/Util.h>
#include <ftk/Core/Math.h>

namespace tl
{
    namespace timeline_gl
    {
        void Render::drawBackground(
            const std::vector<ftk::Box2I>& boxes,
            const ftk::M44F& m,
            const timeline::BackgroundOptions& options)
        {
            const ftk::Box2I rect(ftk::V2I(0, 0), getRenderSize());
            switch (options.type)
            {
            case timeline::Background::Solid:
                IRender::drawRect(rect, options.solidColor);
                break;
            case timeline::Background::Checkers:
                drawColorMesh(
                    ftk::checkers(
                        rect,
                        options.checkersColor.first,
                        options.checkersColor.second,
                        options.checkersSize),
                    ftk::Color4F(1.F, 1.F, 1.F));
                break;
            case timeline::Background::Gradient:
            {
                ftk::TriMesh2F mesh;
                mesh.v.push_back(ftk::V2F(rect.min.x, rect.min.y));
                mesh.v.push_back(ftk::V2F(rect.max.x, rect.min.y));
                mesh.v.push_back(ftk::V2F(rect.max.x, rect.max.y));
                mesh.v.push_back(ftk::V2F(rect.min.x, rect.max.y));
                mesh.c.push_back(ftk::V4F(
                    options.gradientColor.first.r,
                    options.gradientColor.first.g,
                    options.gradientColor.first.b,
                    options.gradientColor.first.a));
                mesh.c.push_back(ftk::V4F(
                    options.gradientColor.second.r,
                    options.gradientColor.second.g,
                    options.gradientColor.second.b,
                    options.gradientColor.second.a));
                mesh.triangles.push_back({
                    ftk::Vertex2(1, 0, 1),
                    ftk::Vertex2(3, 0, 1),
                    ftk::Vertex2(2, 0, 2), });
                mesh.triangles.push_back({
                    ftk::Vertex2(3, 0, 2),
                    ftk::Vertex2(1, 0, 2),
                    ftk::Vertex2(4, 0, 1), });
                drawColorMesh(
                    mesh,
                    ftk::Color4F(1.F, 1.F, 1.F));
                break;
            }
            default: break;
            }
        }

        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<ftk::Box2I>& boxes,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            ftk::ImageType colorBuffer)
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
            const std::vector<ftk::Box2I>& boxes,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            ftk::ImageType colorBuffer)
        {
            if (!videoData.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoData[0],
                    boxes[0],
                    !imageOptions.empty() ? std::make_shared<ftk::ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions(),
                    colorBuffer);
            }
        }

        void Render::_drawVideoB(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<ftk::Box2I>& boxes,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            ftk::ImageType colorBuffer)
        {
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1],
                    boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<ftk::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions(),
                    colorBuffer);
            }
        }

        void Render::_drawVideoWipe(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<ftk::Box2I>& boxes,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            ftk::ImageType colorBuffer)
        {
            FTK_P();

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
            ftk::V2F pts[4];
            for (size_t i = 0; i < 4; ++i)
            {
                float rad = ftk::deg2rad(rotation + 90.F * i + 90.F);
                pts[i].x = cos(rad) * radius + x;
                pts[i].y = sin(rad) * radius + y;
            }

            ftk::gl::SetAndRestore stencilTest(GL_STENCIL_TEST, GL_TRUE);

            const ftk::Size2I renderSize = getRenderSize();
            const ftk::Box2I viewport = getViewport();
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
            p.shaders["wipe"]->setUniform("color", ftk::Color4F(1.F, 0.F, 0.F));
            {
                if (p.vbos["wipe"])
                {
                    ftk::TriMesh2F mesh;
                    mesh.v.push_back(pts[0]);
                    mesh.v.push_back(pts[1]);
                    mesh.v.push_back(pts[2]);
                    ftk::Triangle2 tri;
                    tri.v[0] = 1;
                    tri.v[1] = 3;
                    tri.v[2] = 2;
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
                    !imageOptions.empty() ? std::make_shared<ftk::ImageOptions>(imageOptions[0]) : nullptr,
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
            p.shaders["wipe"]->setUniform("color", ftk::Color4F(0.F, 1.F, 0.F));
            {
                if (p.vbos["wipe"])
                {
                    ftk::TriMesh2F mesh;
                    mesh.v.push_back(pts[2]);
                    mesh.v.push_back(pts[3]);
                    mesh.v.push_back(pts[0]);
                    ftk::Triangle2 tri;
                    tri.v[0] = 1;
                    tri.v[1] = 3;
                    tri.v[2] = 2;
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
                    imageOptions.size() > 1 ? std::make_shared<ftk::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions(),
                    colorBuffer);
            }
        }

        void Render::_drawVideoOverlay(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<ftk::Box2I>& boxes,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            ftk::ImageType colorBuffer)
        {
            FTK_P();

            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1],
                    boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<ftk::ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions(),
                    colorBuffer);
            }
            if (!videoData.empty() && !boxes.empty())
            {
                const ftk::Size2I offscreenBufferSize(
                    boxes[0].w(),
                    boxes[0].h());
                ftk::gl::OffscreenBufferOptions offscreenBufferOptions;
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
                    p.buffers["overlay"] = ftk::gl::OffscreenBuffer::create(
                        offscreenBufferSize,
                        offscreenBufferOptions);
                }

                if (p.buffers["overlay"])
                {
                    const ftk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                    ftk::gl::OffscreenBufferBinding binding(p.buffers["overlay"]);
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
                        ftk::ortho(
                            0.F,
                            static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h),
                            0.F,
                            -1.F,
                            1.F));

                    _drawVideo(
                        videoData[0],
                        ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<ftk::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions(),
                        colorBuffer);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform("transform.mvp", getTransform());
                }

                if (p.buffers["overlay"])
                {
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                    const ftk::Size2I renderSize = getRenderSize();
                    const ftk::Box2I viewport = getViewport();
                    glViewport(
                        viewport.x(),
                        renderSize.h - viewport.h() - viewport.y(),
                        viewport.w(),
                        viewport.h());

                    p.shaders["overlay"]->bind();
                    p.shaders["overlay"]->setUniform("color", ftk::Color4F(1.F, 1.F, 1.F, compareOptions.overlay));
                    p.shaders["overlay"]->setUniform("textureSampler", 0);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    glBindTexture(GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(
                            ftk::mesh(boxes[0]),
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
            const std::vector<ftk::Box2I>& boxes,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            ftk::ImageType colorBuffer)
        {
            FTK_P();
            if (!videoData.empty() && !boxes.empty())
            {
                const ftk::Size2I offscreenBufferSize(
                    boxes[0].w(),
                    boxes[0].h());
                ftk::gl::OffscreenBufferOptions offscreenBufferOptions;
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
                    p.buffers["difference0"] = ftk::gl::OffscreenBuffer::create(
                        offscreenBufferSize,
                        offscreenBufferOptions);
                }

                if (p.buffers["difference0"])
                {
                    const ftk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                    ftk::gl::OffscreenBufferBinding binding(p.buffers["difference0"]);
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
                        ftk::ortho(
                            0.F,
                            static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h),
                            0.F,
                            -1.F,
                            1.F));

                    _drawVideo(
                        videoData[0],
                        ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<ftk::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions(),
                        colorBuffer);

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform("transform.mvp", getTransform());
                }

                if (videoData.size() > 1)
                {
                    offscreenBufferOptions = ftk::gl::OffscreenBufferOptions();
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
                        p.buffers["difference1"] = ftk::gl::OffscreenBuffer::create(
                            offscreenBufferSize,
                            offscreenBufferOptions);
                    }

                    if (p.buffers["difference1"])
                    {
                        const ftk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                        ftk::gl::OffscreenBufferBinding binding(p.buffers["difference1"]);
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
                            ftk::ortho(
                                0.F,
                                static_cast<float>(offscreenBufferSize.w),
                                static_cast<float>(offscreenBufferSize.h),
                                0.F,
                                -1.F,
                                1.F));

                        _drawVideo(
                            videoData[1],
                            ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                            imageOptions.size() > 1 ? std::make_shared<ftk::ImageOptions>(imageOptions[1]) : nullptr,
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

                    const ftk::Size2I renderSize = getRenderSize();
                    const ftk::Box2I viewport = getViewport();
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
                            ftk::mesh(boxes[0]),
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
            const std::vector<ftk::Box2I>& boxes,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions,
            ftk::ImageType colorBuffer)
        {
            for (size_t i = 0; i < videoData.size() && i < boxes.size(); ++i)
            {
                _drawVideo(
                    videoData[i],
                    boxes[i],
                    i < imageOptions.size() ? std::make_shared<ftk::ImageOptions>(imageOptions[i]) : nullptr,
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
            const ftk::Box2I& box,
            const std::shared_ptr<ftk::ImageOptions>& imageOptions,
            const timeline::DisplayOptions& displayOptions,
            ftk::ImageType colorBuffer)
        {
            FTK_P();
            
            GLint viewportPrev[4] = { 0, 0, 0, 0 };
            glGetIntegerv(GL_VIEWPORT, viewportPrev);

            auto imageShader = p.baseRender->getShader("image");
            imageShader->bind();
            const auto transform = ftk::ortho(
                0.F,
                static_cast<float>(box.w()),
                static_cast<float>(box.h()),
                0.F,
                -1.F,
                1.F);
            imageShader->setUniform("transform.mvp", transform);

            const ftk::Size2I& offscreenBufferSize = box.size();
            ftk::gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.color = colorBuffer;
            offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            if (doCreate(
                p.buffers["video"],
                offscreenBufferSize,
                offscreenBufferOptions))
            {
                p.buffers["video"] = ftk::gl::OffscreenBuffer::create(
                    offscreenBufferSize,
                    offscreenBufferOptions);
            }

            if (p.buffers["video"])
            {
                const ftk::gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                ftk::gl::OffscreenBufferBinding binding(p.buffers["video"]);
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
                                    p.buffers["dissolve"] = ftk::gl::OffscreenBuffer::create(
                                        offscreenBufferSize,
                                        offscreenBufferOptions);
                                }
                                if (doCreate(
                                    p.buffers["dissolve2"],
                                    offscreenBufferSize,
                                    offscreenBufferOptions))
                                {
                                    p.buffers["dissolve2"] = ftk::gl::OffscreenBuffer::create(
                                        offscreenBufferSize,
                                        offscreenBufferOptions);
                                }
                                if (p.buffers["dissolve"])
                                {
                                    ftk::gl::OffscreenBufferBinding binding(p.buffers["dissolve"]);
                                    glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                                    glClearColor(0.F, 0.F, 0.F, 0.F);
                                    glClear(GL_COLOR_BUFFER_BIT);
                                    auto dissolveImageOptions = imageOptions.get() ? *imageOptions : layer.imageOptions;
                                    dissolveImageOptions.alphaBlend = ftk::AlphaBlend::None;
                                    IRender::drawImage(
                                        layer.image,
                                        timeline::getBox(
                                            layer.image->getAspect(),
                                            ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                        ftk::Color4F(1.F, 1.F, 1.F),
                                        dissolveImageOptions);
                                }
                                if (p.buffers["dissolve2"])
                                {
                                    ftk::gl::OffscreenBufferBinding binding(p.buffers["dissolve2"]);
                                    glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                                    glClearColor(0.F, 0.F, 0.F, 0.F);
                                    glClear(GL_COLOR_BUFFER_BIT);
                                    auto dissolveImageOptions = imageOptions.get() ? *imageOptions : layer.imageOptionsB;
                                    dissolveImageOptions.alphaBlend = ftk::AlphaBlend::None;
                                    IRender::drawImage(
                                        layer.imageB,
                                        timeline::getBox(
                                            layer.imageB->getAspect(),
                                            ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                        ftk::Color4F(1.F, 1.F, 1.F),
                                        dissolveImageOptions);
                                }
                                if (p.buffers["dissolve"] && p.buffers["dissolve2"])
                                {
                                    p.shaders["dissolve"]->bind();
                                    p.shaders["dissolve"]->setUniform("transform.mvp", transform);
                                    p.shaders["dissolve"]->setUniform("dissolve", layer.transitionValue);
                                    p.shaders["dissolve"]->setUniform("textureSampler", 0);
                                    p.shaders["dissolve"]->setUniform("textureSampler2", 1);

                                    ftk::gl::setAlphaBlend(ftk::AlphaBlend::Straight);

                                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                                    glBindTexture(GL_TEXTURE_2D, p.buffers["dissolve"]->getColorID());
                                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                                    glBindTexture(GL_TEXTURE_2D, p.buffers["dissolve2"]->getColorID());
                                    if (p.vbos["video"])
                                    {
                                        p.vbos["video"]->copy(convert(
                                            ftk::mesh(ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
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
                                        ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                    ftk::Color4F(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptions);
                            }
                            else if (layer.imageB)
                            {
                                IRender::drawImage(
                                    layer.imageB,
                                    timeline::getBox(
                                        layer.imageB->getAspect(),
                                        ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                    ftk::Color4F(1.F, 1.F, 1.F, layer.transitionValue),
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
                                    ftk::Box2I(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                ftk::Color4F(1.F, 1.F, 1.F),
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
                        ftk::mesh(box),
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
            const std::vector<ftk::Box2I>& boxes,
            const ftk::M44F& m,
            const timeline::ForegroundOptions& options)
        {
            if (options.grid.enabled && !boxes.empty())
            {
                const ftk::V3F v0 = m * ftk::V3F(0.F, 0.F, 0.F);
                const ftk::V3F v1 = m * ftk::V3F(options.grid.size, 0.F, 0.F);
                if (ftk::length(v1 - v0) > options.grid.lineWidth + 10.F)
                {
                    ftk::Box2I bounds = boxes.front();
                    for (size_t i = 1; i < boxes.size(); ++i)
                    {
                        bounds.min.x = std::min(bounds.min.x, boxes[i].min.x);
                        bounds.min.y = std::min(bounds.min.y, boxes[i].min.y);
                        bounds.max.x = std::max(bounds.max.x, boxes[i].max.x);
                        bounds.max.y = std::max(bounds.max.y, boxes[i].max.y);
                    }

                    const ftk::Size2I& renderSize = getRenderSize();
                    const ftk::Box2F vp(0, 0, renderSize.w, renderSize.h);
                    std::vector<std::pair<ftk::V2F, ftk::V2F> > lines;
                    ftk::LineOptions lineOptions;
                    lineOptions.width = options.grid.lineWidth;
                    for (int y = bounds.min.y; y <= bounds.max.y + 1; y += options.grid.size)
                    {
                        const ftk::V3F v0 = m * ftk::V3F(bounds.min.x, y, 0.F);
                        const ftk::V3F v1 = m * ftk::V3F(bounds.max.x + 1, y, 0.F);
                        const ftk::V2F v2(std::round(v0.x), std::round(v0.y));
                        const ftk::V2F v3(std::round(v1.x), std::round(v1.y));
                        if (ftk::intersects(ftk::Box2F(v2, v3), vp))
                        {
                            lines.push_back(std::make_pair(v2, v3));
                        }
                    }
                    for (int x = bounds.min.x; x <= bounds.max.x + 1; x += options.grid.size)
                    {
                        const ftk::V3F v0 = m * ftk::V3F(x, bounds.min.y, 0.F);
                        const ftk::V3F v1 = m * ftk::V3F(x, bounds.max.y + 1, 0.F);
                        const ftk::V2F v2(std::round(v0.x), std::round(v0.y));
                        const ftk::V2F v3(std::round(v1.x), std::round(v1.y));
                        if (ftk::intersects(ftk::Box2F(v2, v3), vp))
                        {
                            lines.push_back(std::make_pair(v2, v3));
                        }
                    }
                    drawLines(lines, options.grid.color, lineOptions);
                }
            }

            if (options.outline.enabled && !boxes.empty())
            {
                ftk::Box2I bounds = boxes.front();
                for (size_t i = 1; i < boxes.size(); ++i)
                {
                    bounds.min.x = std::min(bounds.min.x, boxes[i].min.x);
                    bounds.min.y = std::min(bounds.min.y, boxes[i].min.y);
                    bounds.max.x = std::max(bounds.max.x, boxes[i].max.x);
                    bounds.max.y = std::max(bounds.max.y, boxes[i].max.y);
                }

                ftk::TriMesh2F mesh;
                mesh.v.push_back(ftk::V2F(bounds.min.x, bounds.min.y));
                mesh.v.push_back(ftk::V2F(bounds.max.x + 1, bounds.min.y));
                mesh.v.push_back(ftk::V2F(bounds.max.x + 1, bounds.max.y + 1));
                mesh.v.push_back(ftk::V2F(bounds.min.x, bounds.max.y + 1));
                for (auto& v : mesh.v)
                {
                    const ftk::V3F v3 = m * ftk::V3F(v.x, v.y, 0.F);
                    v.x = v3.x;
                    v.y = v3.y;
                }
                mesh.v.push_back(ftk::round(
                    ftk::normalize(mesh.v[0] - mesh.v[1]) * options.outline.width +
                    ftk::normalize(mesh.v[0] - mesh.v[3]) * options.outline.width +
                    mesh.v[0]));
                mesh.v.push_back(ftk::round(
                    ftk::normalize(mesh.v[1] - mesh.v[2]) * options.outline.width +
                    ftk::normalize(mesh.v[1] - mesh.v[0]) * options.outline.width +
                    mesh.v[1]));
                mesh.v.push_back(ftk::round(
                    ftk::normalize(mesh.v[2] - mesh.v[1]) * options.outline.width +
                    ftk::normalize(mesh.v[2] - mesh.v[3]) * options.outline.width +
                    mesh.v[2]));
                mesh.v.push_back(ftk::round(
                    ftk::normalize(mesh.v[3] - mesh.v[0]) * options.outline.width +
                    ftk::normalize(mesh.v[3] - mesh.v[2]) * options.outline.width +
                    mesh.v[3]));
                mesh.triangles.push_back({ ftk::Vertex2(1), ftk::Vertex2(5), ftk::Vertex2(2) });
                mesh.triangles.push_back({ ftk::Vertex2(2), ftk::Vertex2(5), ftk::Vertex2(6) });
                mesh.triangles.push_back({ ftk::Vertex2(2), ftk::Vertex2(6), ftk::Vertex2(3) });
                mesh.triangles.push_back({ ftk::Vertex2(3), ftk::Vertex2(6), ftk::Vertex2(7) });
                mesh.triangles.push_back({ ftk::Vertex2(3), ftk::Vertex2(7), ftk::Vertex2(4) });
                mesh.triangles.push_back({ ftk::Vertex2(4), ftk::Vertex2(7), ftk::Vertex2(8) });
                mesh.triangles.push_back({ ftk::Vertex2(4), ftk::Vertex2(8), ftk::Vertex2(1) });
                mesh.triangles.push_back({ ftk::Vertex2(1), ftk::Vertex2(8), ftk::Vertex2(5) });
                drawMesh(mesh, options.outline.color);
            }
        }
    }
}
