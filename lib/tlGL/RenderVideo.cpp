// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGL/Mesh.h>

namespace tl
{
    namespace gl
    {
        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<timeline::ImageOptions>& imageOptions,
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
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions());
                }
                break;
            case timeline::CompareMode::B:
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        imageOptions.size() > 1 ? imageOptions[1] : timeline::ImageOptions());
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

                glEnable(GL_STENCIL_TEST);

                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                p.shader->bind();
                p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Solid));
                p.shader->setUniform("color", imaging::Color4f(1.F, 0.F, 0.F));
                {
                    std::vector<uint8_t> vboData;
                    vboData.resize(3 * getByteCount(VBOType::Pos2_F32_UV_U16));
                    VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                    vboP[0].vx = pts[0].x;
                    vboP[0].vy = pts[0].y;
                    vboP[0].tx = 0;
                    vboP[0].ty = 0;
                    vboP[1].vx = pts[1].x;
                    vboP[1].vy = pts[1].y;
                    vboP[1].tx = 0;
                    vboP[1].ty = 0;
                    vboP[2].vx = pts[2].x;
                    vboP[2].vy = pts[2].y;
                    vboP[2].tx = 0;
                    vboP[2].ty = 0;
                    auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                    vbo->copy(vboData);

                    auto vao = VAO::create(vbo->getType(), vbo->getID());
                    vao->bind();
                    vao->draw(GL_TRIANGLE_STRIP, 0, 3);
                }
                glStencilFunc(GL_EQUAL, 1, 0xFF);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                if (!videoData.empty())
                {
                    _drawVideo(
                        videoData[0],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions());
                }

                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Solid));
                p.shader->setUniform("color", imaging::Color4f(0.F, 1.F, 0.F));
                {
                    std::vector<uint8_t> vboData;
                    vboData.resize(3 * getByteCount(VBOType::Pos2_F32_UV_U16));
                    VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                    vboP[0].vx = pts[2].x;
                    vboP[0].vy = pts[2].y;
                    vboP[0].tx = 0;
                    vboP[0].ty = 0;
                    vboP[1].vx = pts[3].x;
                    vboP[1].vy = pts[3].y;
                    vboP[1].tx = 0;
                    vboP[1].ty = 0;
                    vboP[2].vx = pts[0].x;
                    vboP[2].vy = pts[0].y;
                    vboP[2].tx = 0;
                    vboP[2].ty = 0;
                    auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                    vbo->copy(vboData);

                    auto vao = VAO::create(vbo->getType(), vbo->getID());
                    vao->bind();
                    vao->draw(GL_TRIANGLE_STRIP, 0, 3);
                }
                glStencilFunc(GL_EQUAL, 1, 0xFF);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        imageOptions.size() > 1 ? imageOptions[1] : timeline::ImageOptions());
                }

                glDisable(GL_STENCIL_TEST);
                break;
            }
            case timeline::CompareMode::Overlay:
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        imageOptions.size() > 1 ? imageOptions[1] : timeline::ImageOptions());
                }
                if (!videoData.empty())
                {
                    if (!p.overlayBuffer ||
                        (p.overlayBuffer && p.overlayBuffer->getSize() != p.size))
                    {
                        OffscreenBufferOptions options;
                        options.colorType = imaging::PixelType::RGBA_F32;
                        p.overlayBuffer = OffscreenBuffer::create(p.size, options);
                    }

                    {
                        auto binding = OffscreenBufferBinding(p.overlayBuffer);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);
                        _drawVideo(
                            videoData[0],
                            math::BBox2i(0, 0, p.size.w, p.size.h),
                            !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions());
                    }

                    p.shader->bind();
                    p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Image));
                    p.shader->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F, compareOptions.overlay));
                    p.shader->setUniform("pixelType", static_cast<int>(imaging::PixelType::RGBA_F32));
                    p.shader->setUniform("textureSampler0", 0);

                    if (p.overlayBuffer)
                    {
                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                        glBindTexture(GL_TEXTURE_2D, p.overlayBuffer->getColorID());
                    }

                    std::vector<uint8_t> vboData;
                    vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
                    VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                    vboP[0].vx = 0.F;
                    vboP[0].vy = 0.F;
                    vboP[0].tx = 0;
                    vboP[0].ty = 65535;
                    vboP[1].vx = p.size.w;
                    vboP[1].vy = 0.F;
                    vboP[1].tx = 65535;
                    vboP[1].ty = 65535;
                    vboP[2].vx = 0.F;
                    vboP[2].vy = p.size.h;
                    vboP[2].tx = 0;
                    vboP[2].ty = 0;
                    vboP[3].vx = p.size.w;
                    vboP[3].vy = p.size.h;
                    vboP[3].tx = 65535;
                    vboP[3].ty = 0;
                    auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                    vbo->copy(vboData);

                    auto vao = VAO::create(vbo->getType(), vbo->getID());
                    vao->bind();
                    vao->draw(GL_TRIANGLE_STRIP, 0, 4);
                }
                break;
            case timeline::CompareMode::Difference:
                if (!videoData.empty())
                {
                    if (!p.differenceBuffers[0] ||
                        (p.differenceBuffers[0] && p.differenceBuffers[0]->getSize() != p.size))
                    {
                        OffscreenBufferOptions options;
                        options.colorType = imaging::PixelType::RGBA_F32;
                        p.differenceBuffers[0] = OffscreenBuffer::create(p.size, options);
                    }

                    if (p.differenceBuffers[0])
                    {
                        auto binding = OffscreenBufferBinding(p.differenceBuffers[0]);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);
                        _drawVideo(
                            videoData[0],
                            math::BBox2i(0, 0, p.size.w, p.size.h),
                            !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions());
                    }

                    if (videoData.size() > 1)
                    {
                        if (!p.differenceBuffers[1] ||
                            (p.differenceBuffers[1] && p.differenceBuffers[1]->getSize() != p.size))
                        {
                            OffscreenBufferOptions options;
                            options.colorType = imaging::PixelType::RGBA_F32;
                            p.differenceBuffers[1] = OffscreenBuffer::create(p.size, options);
                        }

                        if (p.differenceBuffers[1])
                        {
                            auto binding = OffscreenBufferBinding(p.differenceBuffers[1]);
                            glClearColor(0.F, 0.F, 0.F, 0.F);
                            glClear(GL_COLOR_BUFFER_BIT);
                            _drawVideo(
                                videoData[1],
                                math::BBox2i(0, 0, p.size.w, p.size.h),
                                !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions());
                        }
                    }

                    p.differenceShader->bind();
                    p.differenceShader->setUniform("textureSampler0", 0);
                    p.differenceShader->setUniform("textureSampler1", 1);

                    if (p.differenceBuffers[0])
                    {
                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                        glBindTexture(GL_TEXTURE_2D, p.differenceBuffers[0]->getColorID());
                    }
                    if (p.differenceBuffers[1])
                    {
                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                        glBindTexture(GL_TEXTURE_2D, p.differenceBuffers[1]->getColorID());
                    }

                    std::vector<uint8_t> vboData;
                    vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
                    VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                    vboP[0].vx = 0.F;
                    vboP[0].vy = 0.F;
                    vboP[0].tx = 0;
                    vboP[0].ty = 65535;
                    vboP[1].vx = p.size.w;
                    vboP[1].vy = 0.F;
                    vboP[1].tx = 65535;
                    vboP[1].ty = 65535;
                    vboP[2].vx = 0.F;
                    vboP[2].vy = p.size.h;
                    vboP[2].tx = 0;
                    vboP[2].ty = 0;
                    vboP[3].vx = p.size.w;
                    vboP[3].vy = p.size.h;
                    vboP[3].tx = 65535;
                    vboP[3].ty = 0;
                    auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                    vbo->copy(vboData);

                    auto vao = VAO::create(vbo->getType(), vbo->getID());
                    vao->bind();
                    vao->draw(GL_TRIANGLE_STRIP, 0, 4);
                }
                break;
            case timeline::CompareMode::Horizontal:
            case timeline::CompareMode::Vertical:
            case timeline::CompareMode::Tile:
            {
                std::vector<imaging::Size> sizes;
                for (const auto& v : videoData)
                {
                    if (!v.layers.empty())
                    {
                        sizes.push_back(v.layers[0].image->getSize());
                    }
                }
                const auto tiles = timeline::tiles(compareOptions.mode, sizes);
                for (size_t i = 0; i < tiles.size() && i < videoData.size(); ++i)
                {
                    _drawVideo(
                        videoData[i],
                        tiles[i],
                        i < imageOptions.size() ? imageOptions[i] : timeline::ImageOptions());
                }
                break;
            }
            default: break;
            }
        }

        void Render::_drawVideo(
            const timeline::VideoData& videoData,
            const math::BBox2i& bbox,
            const timeline::ImageOptions& imageOptions)
        {
            TLRENDER_P();
            for (const auto& layer : videoData.layers)
            {
                switch (layer.transition)
                {
                case timeline::Transition::Dissolve:
                {
                    if (!p.dissolveBuffer ||
                        (p.dissolveBuffer && p.dissolveBuffer->getSize() != p.size))
                    {
                        OffscreenBufferOptions options;
                        options.colorType = imaging::PixelType::RGBA_F32;
                        p.dissolveBuffer = OffscreenBuffer::create(p.size, options);
                    }

                    {
                        auto binding = OffscreenBufferBinding(p.dissolveBuffer);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
                        timeline::ImageOptions imageOptionsTmp;
                        imageOptionsTmp.yuvRange = imageOptions.yuvRange;
                        if (layer.image)
                        {
                            const float t = 1.F - layer.transitionValue;
                            drawImage(
                                layer.image,
                                imaging::getBBox(layer.image->getAspect(), bbox),
                                imaging::Color4f(t, t, t, t),
                                imageOptionsTmp);
                        }
                        if (layer.imageB)
                        {
                            const float tB = layer.transitionValue;
                            drawImage(
                                layer.imageB,
                                imaging::getBBox(layer.imageB->getAspect(), bbox),
                                imaging::Color4f(tB, tB, tB, tB),
                                imageOptionsTmp);
                        }
                        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }

                    p.shader->bind();
                    p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Image));
                    p.shader->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F));
                    p.shader->setUniform("pixelType", static_cast<int>(imaging::PixelType::RGBA_F32));
                    p.shader->setUniform("textureSampler0", 0);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    glBindTexture(GL_TEXTURE_2D, p.dissolveBuffer->getColorID());

                    std::vector<uint8_t> vboData;
                    vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
                    VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                    vboP[0].vx = 0.F;
                    vboP[0].vy = 0.F;
                    vboP[0].tx = 0;
                    vboP[0].ty = 65535;
                    vboP[1].vx = p.size.w;
                    vboP[1].vy = 0.F;
                    vboP[1].tx = 65535;
                    vboP[1].ty = 65535;
                    vboP[2].vx = 0.F;
                    vboP[2].vy = p.size.h;
                    vboP[2].tx = 0;
                    vboP[2].ty = 0;
                    vboP[3].vx = p.size.w;
                    vboP[3].vy = p.size.h;
                    vboP[3].tx = 65535;
                    vboP[3].ty = 0;
                    auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                    vbo->copy(vboData);

                    auto vao = VAO::create(vbo->getType(), vbo->getID());
                    vao->bind();
                    vao->draw(GL_TRIANGLE_STRIP, 0, 4);

                    break;
                }
                default:
                    if (layer.image)
                    {
                        drawImage(
                            layer.image,
                            imaging::getBBox(layer.image->getAspect(), bbox),
                            imaging::Color4f(1.F, 1.F, 1.F),
                            imageOptions);
                    }
                    break;
                }
            }
        }
    }
}
