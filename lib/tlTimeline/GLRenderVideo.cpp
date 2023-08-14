// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/GLRenderPrivate.h>

#include <tlGL/GL.h>
#include <tlGL/Mesh.h>
#include <tlGL/Util.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace timeline
    {
        void GLRender::drawVideo(
            const std::vector<VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions)
        {
            switch (compareOptions.mode)
            {
            case CompareMode::A:
                _drawVideoA(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions);
                break;
            case CompareMode::B:
                _drawVideoB(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions);
                break;
            case CompareMode::Wipe:
                _drawVideoWipe(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions);
                break;
            case CompareMode::Overlay:
                _drawVideoOverlay(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions);
                break;
            case CompareMode::Difference:
                if (videoData.size() > 1)
                {
                    _drawVideoDifference(
                        videoData,
                        boxes,
                        imageOptions,
                        displayOptions,
                        compareOptions);
                }
                else
                {
                    _drawVideoA(
                        videoData,
                        boxes,
                        imageOptions,
                        displayOptions,
                        compareOptions);
                }
                break;
            case CompareMode::Horizontal:
            case CompareMode::Vertical:
            case CompareMode::Tile:
                _drawVideoTile(
                    videoData,
                    boxes,
                    imageOptions,
                    displayOptions,
                    compareOptions);
                break;
            default: break;
            }
        }

        void GLRender::_drawVideoA(
            const std::vector<VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions)
        {
            if (!videoData.empty() && !boxes.empty())
            {
                _drawVideo(
                    videoData[0],
                    boxes[0],
                    !imageOptions.empty() ? std::make_shared<ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : DisplayOptions());
            }
        }

        void GLRender::_drawVideoB(
            const std::vector<VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions)
        {
            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1],
                    boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : DisplayOptions());
            }
        }

        void GLRender::_drawVideoWipe(
            const std::vector<VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions)
        {
            TLRENDER_P();

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
            math::Vector2f pts[4];
            for (size_t i = 0; i < 4; ++i)
            {
                float rad = math::deg2rad(rotation + 90.F * i + 90.F);
                pts[i].x = cos(rad) * radius + x;
                pts[i].y = sin(rad) * radius + y;
            }

            gl::SetAndRestore stencilTest(GL_STENCIL_TEST, GL_TRUE);

            glViewport(
                p.viewport.x(),
                p.renderSize.h - p.viewport.h() - p.viewport.y(),
                p.viewport.w(),
                p.viewport.h());
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind();
            p.shaders["wipe"]->setUniform("color", image::Color4f(1.F, 0.F, 0.F));
            {
                if (p.vbos["wipe"])
                {
                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(pts[0]);
                    mesh.v.push_back(pts[1]);
                    mesh.v.push_back(pts[2]);
                    geom::Triangle2 tri;
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
                    !imageOptions.empty() ? std::make_shared<ImageOptions>(imageOptions[0]) : nullptr,
                    !displayOptions.empty() ? displayOptions[0] : DisplayOptions());
            }

            glViewport(
                p.viewport.x(),
                p.renderSize.h - p.viewport.h() - p.viewport.y(),
                p.viewport.w(),
                p.viewport.h());
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            p.shaders["wipe"]->bind();
            p.shaders["wipe"]->setUniform("color", image::Color4f(0.F, 1.F, 0.F));
            {
                if (p.vbos["wipe"])
                {
                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(pts[2]);
                    mesh.v.push_back(pts[3]);
                    mesh.v.push_back(pts[0]);
                    geom::Triangle2 tri;
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
                    imageOptions.size() > 1 ? std::make_shared<ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : DisplayOptions());
            }
        }

        void GLRender::_drawVideoOverlay(
            const std::vector<VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions)
        {
            TLRENDER_P();

            if (videoData.size() > 1 && boxes.size() > 1)
            {
                _drawVideo(
                    videoData[1],
                    boxes[1],
                    imageOptions.size() > 1 ? std::make_shared<ImageOptions>(imageOptions[1]) : nullptr,
                    displayOptions.size() > 1 ? displayOptions[1] : DisplayOptions());
            }
            if (!videoData.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(
                    boxes[0].w(),
                    boxes[0].h());
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                if (!displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                if (doCreate(
                    p.buffers["overlay"],
                    offscreenBufferSize,
                    offscreenBufferOptions))
                {
                    p.buffers["overlay"] = gl::OffscreenBuffer::create(
                        offscreenBufferSize,
                        offscreenBufferOptions);
                }

                if (p.buffers["overlay"])
                {
                    const gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                    gl::OffscreenBufferBinding binding(p.buffers["overlay"]);
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
                        math::ortho(
                            0.F,
                            static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h),
                            0.F,
                            -1.F,
                            1.F));

                    _drawVideo(
                        videoData[0],
                        math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : DisplayOptions());

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform("transform.mvp", p.transform);
                }

                if (p.buffers["overlay"])
                {
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                    glViewport(
                        p.viewport.x(),
                        p.renderSize.h - p.viewport.h() - p.viewport.y(),
                        p.viewport.w(),
                        p.viewport.h());

                    p.shaders["overlay"]->bind();
                    p.shaders["overlay"]->setUniform("color", image::Color4f(1.F, 1.F, 1.F, compareOptions.overlay));
                    p.shaders["overlay"]->setUniform("textureSampler", 0);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    glBindTexture(GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());

                    if (p.vbos["video"])
                    {
                        p.vbos["video"]->copy(convert(
                            geom::box(boxes[0], true),
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

        void GLRender::_drawVideoDifference(
            const std::vector<VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions)
        {
            TLRENDER_P();
            if (!videoData.empty() && !boxes.empty())
            {
                const math::Size2i offscreenBufferSize(
                    boxes[0].w(),
                    boxes[0].h());
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                if (!imageOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                }
                if (doCreate(
                    p.buffers["difference0"],
                    offscreenBufferSize,
                    offscreenBufferOptions))
                {
                    p.buffers["difference0"] = gl::OffscreenBuffer::create(
                        offscreenBufferSize,
                        offscreenBufferOptions);
                }

                if (p.buffers["difference0"])
                {
                    const gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                    gl::OffscreenBufferBinding binding(p.buffers["difference0"]);
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
                        math::ortho(
                            0.F,
                            static_cast<float>(offscreenBufferSize.w),
                            static_cast<float>(offscreenBufferSize.h),
                            0.F,
                            -1.F,
                            1.F));

                    _drawVideo(
                        videoData[0],
                        math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                        !imageOptions.empty() ? std::make_shared<ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : DisplayOptions());

                    p.shaders["display"]->bind();
                    p.shaders["display"]->setUniform("transform.mvp", p.transform);
                }

                if (videoData.size() > 1)
                {
                    offscreenBufferOptions = gl::OffscreenBufferOptions();
                    offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                    if (imageOptions.size() > 1)
                    {
                        offscreenBufferOptions.colorFilters = displayOptions[1].imageFilters;
                    }
                    if (doCreate(
                        p.buffers["difference1"],
                        offscreenBufferSize,
                        offscreenBufferOptions))
                    {
                        p.buffers["difference1"] = gl::OffscreenBuffer::create(
                            offscreenBufferSize,
                            offscreenBufferOptions);
                    }

                    if (p.buffers["difference1"])
                    {
                        const gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                        gl::OffscreenBufferBinding binding(p.buffers["difference1"]);
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
                            math::ortho(
                                0.F,
                                static_cast<float>(offscreenBufferSize.w),
                                static_cast<float>(offscreenBufferSize.h),
                                0.F,
                                -1.F,
                                1.F));

                        _drawVideo(
                            videoData[1],
                            math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h),
                            imageOptions.size() > 1 ? std::make_shared<ImageOptions>(imageOptions[1]) : nullptr,
                            displayOptions.size() > 1 ? displayOptions[1] : DisplayOptions());
                    }
                }
                else
                {
                    p.buffers["difference1"].reset();
                }

                if (p.buffers["difference0"] && p.buffers["difference1"])
                {
                    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                    glViewport(
                        p.viewport.x(),
                        p.renderSize.h - p.viewport.h() - p.viewport.y(),
                        p.viewport.w(),
                        p.viewport.h());

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
                            geom::box(boxes[0], true),
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

        void GLRender::_drawVideoTile(
            const std::vector<VideoData>& videoData,
            const std::vector<math::Box2i>& boxes,
            const std::vector<ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            const CompareOptions& compareOptions)
        {
            for (size_t i = 0; i < videoData.size() && i < boxes.size(); ++i)
            {
                _drawVideo(
                    videoData[i],
                    boxes[i],
                    i < imageOptions.size() ? std::make_shared<ImageOptions>(imageOptions[i]) : nullptr,
                    i < displayOptions.size() ? displayOptions[i] : DisplayOptions());
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

        void GLRender::_drawVideo(
            const VideoData& videoData,
            const math::Box2i& box,
            const std::shared_ptr<ImageOptions>& imageOptions,
            const DisplayOptions& displayOptions)
        {
            TLRENDER_P();
            
            GLint viewportPrev[4] = { 0, 0, 0, 0 };
            glGetIntegerv(GL_VIEWPORT, viewportPrev);

            const auto transform = math::ortho(
                0.F,
                static_cast<float>(box.w()),
                static_cast<float>(box.h()),
                0.F,
                -1.F,
                1.F);
            p.shaders["image"]->bind();
            p.shaders["image"]->setUniform("transform.mvp", transform);

            const math::Size2i& offscreenBufferSize = box.getSize();
            gl::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
            if (imageOptions.get())
            {
                offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            }
            if (doCreate(
                p.buffers["video"],
                offscreenBufferSize,
                offscreenBufferOptions))
            {
                p.buffers["video"] = gl::OffscreenBuffer::create(
                    offscreenBufferSize,
                    offscreenBufferOptions);
            }

            if (p.buffers["video"])
            {
                const gl::SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);

                gl::OffscreenBufferBinding binding(p.buffers["video"]);
                glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                for (const auto& layer : videoData.layers)
                {
                    switch (layer.transition)
                    {
                        case Transition::Dissolve:
                        {
                            if (layer.image && layer.imageB)
                            {
                                if (doCreate(
                                    p.buffers["dissolve"],
                                    offscreenBufferSize,
                                    offscreenBufferOptions))
                                {
                                    p.buffers["dissolve"] = gl::OffscreenBuffer::create(
                                        offscreenBufferSize,
                                        offscreenBufferOptions);
                                }
                                if (p.buffers["dissolve"])
                                {
                                    gl::OffscreenBufferBinding binding(p.buffers["dissolve"]);
                                    glViewport(0, 0, offscreenBufferSize.w, offscreenBufferSize.h);
                                    glClearColor(0.F, 0.F, 0.F, 0.F);
                                    glClear(GL_COLOR_BUFFER_BIT);

                                    drawImage(
                                        layer.image,
                                        image::getBox(
                                            layer.image->getAspect(),
                                            math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                        image::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                        imageOptions.get() ? *imageOptions : layer.imageOptions);
                                    drawImage(
                                        layer.imageB,
                                        image::getBox(
                                            layer.imageB->getAspect(),
                                            math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                        image::Color4f(1.F, 1.F, 1.F, layer.transitionValue),
                                        imageOptions.get() ? *imageOptions : layer.imageOptionsB);
                                }
                                if (p.buffers["dissolve"])
                                {
                                    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                                    p.shaders["dissolve"]->bind();
                                    p.shaders["dissolve"]->setUniform("transform.mvp", transform);
                                    p.shaders["dissolve"]->setUniform("color", image::Color4f(1.F, 1.F, 1.F));
                                    p.shaders["dissolve"]->setUniform("textureSampler", 0);

                                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                                    glBindTexture(GL_TEXTURE_2D, p.buffers["dissolve"]->getColorID());

                                    if (p.vbos["video"])
                                    {
                                        p.vbos["video"]->copy(convert(
                                            geom::box(math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h), true),
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
                                drawImage(
                                    layer.image,
                                    image::getBox(
                                        layer.image->getAspect(),
                                        math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                    image::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptions);
                            }
                            else if (layer.imageB)
                            {
                                drawImage(
                                    layer.imageB,
                                    image::getBox(
                                        layer.imageB->getAspect(),
                                        math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                    image::Color4f(1.F, 1.F, 1.F, layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptionsB);
                            }
                            break;
                        }
                    default:
                        if (layer.image)
                        {
                            drawImage(
                                layer.image,
                                image::getBox(
                                    layer.image->getAspect(),
                                    math::Box2i(0, 0, offscreenBufferSize.w, offscreenBufferSize.h)),
                                image::Color4f(1.F, 1.F, 1.F),
                                imageOptions.get() ? *imageOptions : layer.imageOptions);
                        }
                        break;
                    }
                }
            }

            if (p.buffers["video"])
            {
                glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

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
                    displayOptions.color != Color() &&
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
                if (p.colorConfigData)
                {
                    for (size_t i = 0; i < p.colorConfigData->textures.size(); ++i)
                    {
                        glActiveTexture(GL_TEXTURE0 + texturesOffset + i);
                        glBindTexture(
                            p.colorConfigData->textures[i].type,
                            p.colorConfigData->textures[i].id);
                    }
                    texturesOffset += p.colorConfigData->textures.size();
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
                        geom::box(box, true),
                        p.vbos["video"]->getType()));
                }
                if (p.vaos["video"])
                {
                    p.vaos["video"]->bind();
                    p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                }
            }

            p.shaders["image"]->bind();
            p.shaders["image"]->setUniform("transform.mvp", p.transform);
        }
    }
}
