// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGL/Mesh.h>
#include <tlGL/State.h>
#include <tlGL/Util.h>

#include <tlCore/Math.h>

#include <tlGlad/gl.h>

namespace tl
{
    namespace gl
    {
        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::BBox2i>& bbox,
            const std::vector<timeline::ImageOptions>& imageOptions,
            const std::vector<timeline::DisplayOptions>& displayOptions,
            const timeline::CompareOptions& compareOptions)
        {
            TLRENDER_P();

            switch (compareOptions.mode)
            {
            case timeline::CompareMode::A:
                if (!videoData.empty())
                {
                    _drawVideo(
                        videoData[0],
                        bbox[0],
                        !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                }
                break;
            case timeline::CompareMode::B:
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        bbox[1],
                        imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                        displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                }
                break;
            case timeline::CompareMode::Wipe:
            {
                const float radius = std::max(p.size.w, p.size.h) * 2.5F;
                const float x = p.size.w * compareOptions.wipeCenter.x;
                const float y = p.size.h * compareOptions.wipeCenter.y;
                const float rotation = compareOptions.wipeRotation;
                math::Vector2f pts[4];
                for (size_t i = 0; i < 4; ++i)
                {
                    float rad = math::deg2rad(rotation + 90.F * i + 90.F);
                    pts[i].x = cos(rad) * radius + x;
                    pts[i].y = sin(rad) * radius + y;
                }

                SetAndRestore stencilTest(GL_STENCIL_TEST, GL_TRUE);

                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                p.shaders["mesh"]->bind();
                p.shaders["mesh"]->setUniform("color", imaging::Color4f(1.F, 0.F, 0.F));
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
                if (!videoData.empty())
                {
                    _drawVideo(
                        videoData[0],
                        bbox[0],
                        !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                }

                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                p.shaders["mesh"]->bind();
                p.shaders["mesh"]->setUniform("color", imaging::Color4f(0.F, 1.F, 0.F));
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
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        bbox[1],
                        imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                        displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                }
                break;
            }
            case timeline::CompareMode::Overlay:
            {
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        bbox[1],
                        imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                        displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                }
                if (!videoData.empty())
                {
                    OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                    if (!displayOptions.empty())
                    {
                        offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                    }
                    if (doCreate(p.buffers["overlay"], p.size, offscreenBufferOptions))
                    {
                        p.buffers["overlay"] = OffscreenBuffer::create(p.size, offscreenBufferOptions);
                    }

                    if (p.buffers["overlay"])
                    {
                        OffscreenBufferBinding binding(p.buffers["overlay"]);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);

                        _drawVideo(
                            videoData[0],
                            bbox[0],
                            !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                            !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                    }

                    if (p.buffers["overlay"])
                    {
                        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                        p.shaders["texture"]->bind();
                        p.shaders["texture"]->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F, compareOptions.overlay));
                        p.shaders["texture"]->setUniform("textureSampler", 0);

                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                        glBindTexture(GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());

                        if (p.vbos["video"])
                        {
                            p.vbos["video"]->copy(convert(
                                geom::bbox(math::BBox2i(0, 0, p.size.w, p.size.h), true),
                                p.vbos["video"]->getType()));
                        }
                        if (p.vaos["video"])
                        {
                            p.vaos["video"]->bind();
                            p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                        }
                    }
                }
                break;
            }
            case timeline::CompareMode::Difference:
                if (!videoData.empty())
                {
                    OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                    if (!imageOptions.empty())
                    {
                        offscreenBufferOptions.colorFilters = displayOptions[0].imageFilters;
                    }
                    if (doCreate(p.buffers["difference0"], p.size, offscreenBufferOptions))
                    {
                        p.buffers["difference0"] = OffscreenBuffer::create(p.size, offscreenBufferOptions);
                    }

                    if (p.buffers["difference0"])
                    {
                        OffscreenBufferBinding binding(p.buffers["difference0"]);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);

                        _drawVideo(
                            videoData[0],
                            bbox[0],
                            !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                            !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                    }

                    if (videoData.size() > 1)
                    {
                        offscreenBufferOptions = OffscreenBufferOptions();
                        offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                        if (imageOptions.size() > 1)
                        {
                            offscreenBufferOptions.colorFilters = displayOptions[1].imageFilters;
                        }
                        if (doCreate(p.buffers["difference1"], p.size, offscreenBufferOptions))
                        {
                            p.buffers["difference1"] = OffscreenBuffer::create(p.size, offscreenBufferOptions);
                        }

                        if (p.buffers["difference1"])
                        {
                            OffscreenBufferBinding binding(p.buffers["difference1"]);
                            glClearColor(0.F, 0.F, 0.F, 0.F);
                            glClear(GL_COLOR_BUFFER_BIT);

                            _drawVideo(
                                videoData[1],
                                bbox[1],
                                imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                                displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                        }
                    }

                    if (p.buffers["difference0"] && p.buffers["difference1"])
                    {
                        glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

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
                                geom::bbox(math::BBox2i(0, 0, p.size.w, p.size.h), true),
                                p.vbos["video"]->getType()));
                        }
                        if (p.vaos["video"])
                        {
                            p.vaos["video"]->bind();
                            p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                        }
                    }
                }
                break;
            case timeline::CompareMode::Horizontal:
            case timeline::CompareMode::Vertical:
            case timeline::CompareMode::Tile:
            {
                for (size_t i = 0; i < videoData.size(); ++i)
                {
                    _drawVideo(
                        videoData[i],
                        bbox[i],
                        i < imageOptions.size() ? std::make_shared<timeline::ImageOptions>(imageOptions[i]) : nullptr,
                        i < displayOptions.size() ? displayOptions[i] : timeline::DisplayOptions());
                }
                break;
            }
            default: break;
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
            const math::BBox2i& bbox,
            const std::shared_ptr<timeline::ImageOptions>& imageOptions,
            const timeline::DisplayOptions& displayOptions)
        {
            TLRENDER_P();

            OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
            if (imageOptions.get())
            {
                offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            }
            if (doCreate(p.buffers["video"], p.size, offscreenBufferOptions))
            {
                p.buffers["video"] = OffscreenBuffer::create(p.size, offscreenBufferOptions);
            }

            if (p.buffers["video"])
            {
                OffscreenBufferBinding binding(p.buffers["video"]);
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
                                if (doCreate(p.buffers["dissolve"], p.size, offscreenBufferOptions))
                                {
                                    p.buffers["dissolve"] = OffscreenBuffer::create(p.size, offscreenBufferOptions);
                                }
                                if (p.buffers["dissolve"])
                                {
                                    OffscreenBufferBinding binding(p.buffers["dissolve"]);
                                    glClearColor(0.F, 0.F, 0.F, 0.F);
                                    glClear(GL_COLOR_BUFFER_BIT);

                                    drawImage(
                                        layer.image,
                                        imaging::getBBox(layer.image->getAspect(), bbox),
                                        imaging::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                        imageOptions.get() ? *imageOptions : layer.imageOptions);
                                    drawImage(
                                        layer.imageB,
                                        imaging::getBBox(layer.imageB->getAspect(), bbox),
                                        imaging::Color4f(1.F, 1.F, 1.F, layer.transitionValue),
                                        imageOptions.get() ? *imageOptions : layer.imageOptionsB);
                                }
                                if (p.buffers["dissolve"])
                                {
                                    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                                    p.shaders["texture"]->bind();
                                    p.shaders["texture"]->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F));
                                    p.shaders["texture"]->setUniform("textureSampler", 0);

                                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                                    glBindTexture(GL_TEXTURE_2D, p.buffers["dissolve"]->getColorID());

                                    if (p.vbos["video"])
                                    {
                                        p.vbos["video"]->copy(convert(
                                            geom::bbox(math::BBox2i(0, 0, p.size.w, p.size.h), true),
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
                                    imaging::getBBox(layer.image->getAspect(), bbox),
                                    imaging::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptions);
                            }
                            else if (layer.imageB)
                            {
                                drawImage(
                                    layer.imageB,
                                    imaging::getBBox(layer.imageB->getAspect(), bbox),
                                    imaging::Color4f(1.F, 1.F, 1.F, layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptionsB);
                            }
                            break;
                        }
                    default:
                        if (layer.image)
                        {
                            drawImage(
                                layer.image,
                                imaging::getBBox(layer.image->getAspect(), bbox),
                                imaging::Color4f(1.F, 1.F, 1.F),
                                imageOptions.get() ? *imageOptions : layer.imageOptions);
                        }
                        break;
                    }
                }
            }

            if (p.buffers["video"])
            {
                glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

                p.shaders["display"]->bind();
                p.shaders["display"]->setUniform("textureSampler", 0);
                p.shaders["display"]->setUniform("channels", static_cast<int>(displayOptions.channels));
                p.shaders["display"]->setUniform("mirrorX", displayOptions.mirror.x);
                p.shaders["display"]->setUniform("mirrorY", displayOptions.mirror.y);
                const bool colorMatrixEnabled = displayOptions.colorEnabled && displayOptions.color != timeline::Color();
                p.shaders["display"]->setUniform("colorEnabled", colorMatrixEnabled);
                p.shaders["display"]->setUniform("colorAdd", displayOptions.color.add);
                if (colorMatrixEnabled)
                {
                    p.shaders["display"]->setUniform("colorMatrix", timeline::color(displayOptions.color));
                }
                p.shaders["display"]->setUniform("colorInvert", displayOptions.colorEnabled ? displayOptions.color.invert : false);
                p.shaders["display"]->setUniform("levelsEnabled", displayOptions.levelsEnabled);
                p.shaders["display"]->setUniform("levels.inLow", displayOptions.levels.inLow);
                p.shaders["display"]->setUniform("levels.inHigh", displayOptions.levels.inHigh);
                p.shaders["display"]->setUniform("levels.gamma", displayOptions.levels.gamma > 0.F ? (1.F / displayOptions.levels.gamma) : 1000000.F);
                p.shaders["display"]->setUniform("levels.outLow", displayOptions.levels.outLow);
                p.shaders["display"]->setUniform("levels.outHigh", displayOptions.levels.outHigh);
                p.shaders["display"]->setUniform("exrDisplayEnabled", displayOptions.exrDisplayEnabled);
                if (displayOptions.exrDisplayEnabled)
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
                    const float gamma = displayOptions.levels.gamma > 0.F ? (1.F / displayOptions.levels.gamma) : 1000000.F;
                    p.shaders["display"]->setUniform("exrDisplay.g", gamma );
                }
                p.shaders["display"]->setUniform("softClip", displayOptions.softClipEnabled ? displayOptions.softClip : 0.F);
                p.shaders["display"]->setUniform("videoLevels", static_cast<int>(displayOptions.videoLevels));

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
                        geom::bbox(math::BBox2i(0, 0, p.size.w, p.size.h), true),
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
}
