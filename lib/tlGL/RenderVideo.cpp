// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGL/Mesh.h>

#include <glm/gtc/matrix_transform.hpp>

namespace tl
{
    namespace gl
    {
        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<math::BBox2i>& bboxes,
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
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions(),
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                }
                break;
            case timeline::CompareMode::B:
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        imageOptions.size() > 1 ? imageOptions[1] : timeline::ImageOptions(),
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

                glEnable(GL_STENCIL_TEST);

                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                p.rectShader->bind();
                p.rectShader->setUniform("color", imaging::Color4f(1.F, 0.F, 0.F));
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
                        !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions(),
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                }

                glClear(GL_STENCIL_BUFFER_BIT);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                p.rectShader->bind();
                p.rectShader->setUniform("color", imaging::Color4f(0.F, 1.F, 0.F));
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
                        imageOptions.size() > 1 ? imageOptions[1] : timeline::ImageOptions(),
                        !displayOptions.empty() ? displayOptions[1] : timeline::DisplayOptions());
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
                        imageOptions.size() > 1 ? imageOptions[1] : timeline::ImageOptions(),
                        displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
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

                    if (p.overlayBuffer)
                    {
                        auto binding = OffscreenBufferBinding(p.overlayBuffer);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);
                        _drawVideo(
                            videoData[0],
                            math::BBox2i(0, 0, p.size.w, p.size.h),
                            !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions(),
                            !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                    }

                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    p.textureShader->bind();
                    p.textureShader->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F, compareOptions.overlay));
                    p.textureShader->setUniform("textureSampler", 0);

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
                            !imageOptions.empty() ? imageOptions[0] : timeline::ImageOptions(),
                            !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
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
                                imageOptions.size() > 1 ? imageOptions[1] : timeline::ImageOptions(),
                                displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                        }
                    }

                    p.differenceShader->bind();
                    p.differenceShader->setUniform("textureSampler", 0);
                    p.differenceShader->setUniform("textureSamplerB", 1);

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
                for (size_t i = 0; i < bboxes.size() && i < videoData.size(); ++i)
                {
                    _drawVideo(
                        videoData[i],
                        bboxes[i],
                        i < imageOptions.size() ? imageOptions[i] : timeline::ImageOptions(),
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
            const timeline::ImageOptions& imageOptions,
            const timeline::DisplayOptions& displayOptions)
        {
            TLRENDER_P();

            const imaging::Size size(bbox.w(), bbox.h());
            if (!p.buffer ||
                (p.buffer && p.buffer->getSize() != size))
            {
                OffscreenBufferOptions options;
                options.colorType = imaging::PixelType::RGBA_F32;
                p.buffer = OffscreenBuffer::create(size, options);
            }

            if (p.buffer)
            {
                auto binding = OffscreenBufferBinding(p.buffer);
                glViewport(0, 0, size.w, size.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                const auto viewMatrix = glm::ortho(
                    0.F,
                    static_cast<float>(size.w),
                    static_cast<float>(size.h),
                    0.F,
                    -1.F,
                    1.F);
                const math::Matrix4x4f mvp(
                    viewMatrix[0][0], viewMatrix[0][1], viewMatrix[0][2], viewMatrix[0][3],
                    viewMatrix[1][0], viewMatrix[1][1], viewMatrix[1][2], viewMatrix[1][3],
                    viewMatrix[2][0], viewMatrix[2][1], viewMatrix[2][2], viewMatrix[2][3],
                    viewMatrix[3][0], viewMatrix[3][1], viewMatrix[3][2], viewMatrix[3][3]);

                for (const auto& layer : videoData.layers)
                {
                    switch (layer.transition)
                    {
                        case timeline::Transition::Dissolve:
                        {
                            if (layer.image && layer.imageB)
                            {
                                p.dissolveShader->bind();
                                p.dissolveShader->setUniform("transform.mvp", mvp);
                                p.dissolveShader->setUniform("transition", layer.transitionValue);

                                const auto& info = layer.image->getInfo();
                                p.dissolveShader->setUniform("pixelType", static_cast<int>(layer.image->getPixelType()));
                                imaging::YUVRange yuvRange = info.yuvRange;
                                switch (imageOptions.yuvRange)
                                {
                                case timeline::YUVRange::Full:  yuvRange = imaging::YUVRange::Full;  break;
                                case timeline::YUVRange::Video: yuvRange = imaging::YUVRange::Video; break;
                                default: break;
                                }
                                p.dissolveShader->setUniform("yuvRange", static_cast<int>(yuvRange));
                                p.dissolveShader->setUniform("imageChannels", imaging::getChannelCount(info.pixelType));
                                p.dissolveShader->setUniform("flipX", info.layout.mirror.x);
                                p.dissolveShader->setUniform("flipY", info.layout.mirror.y);
                                math::BBox2i bbox2 = imaging::getBBox(layer.image->getAspect(), math::BBox2i(0, 0, size.w, size.h));
                                math::BBox2f textureRange(
                                    .5F - bbox2.w() / static_cast<float>(size.w) / 2.F,
                                    .5F - bbox2.h() / static_cast<float>(size.h) / 2.F,
                                    (bbox2.w() - 1) / static_cast<float>(size.w - 1),
                                    (bbox2.h() - 1) / static_cast<float>(size.h - 1));
                                p.dissolveShader->setUniform("textureRangeU", math::Vector2f(textureRange.min.x, textureRange.max.x));
                                p.dissolveShader->setUniform("textureRangeV", math::Vector2f(textureRange.min.y, textureRange.max.y));
                                p.dissolveShader->setUniform("textureSampler0", 0);
                                p.dissolveShader->setUniform("textureSampler1", 1);
                                p.dissolveShader->setUniform("textureSampler2", 2);

                                const auto& infoB = layer.imageB->getInfo();
                                p.dissolveShader->setUniform("pixelTypeB", static_cast<int>(layer.imageB->getPixelType()));
                                yuvRange = infoB.yuvRange;
                                switch (imageOptions.yuvRange)
                                {
                                case timeline::YUVRange::Full:  yuvRange = imaging::YUVRange::Full;  break;
                                case timeline::YUVRange::Video: yuvRange = imaging::YUVRange::Video; break;
                                default: break;
                                }
                                p.dissolveShader->setUniform("yuvRangeB", static_cast<int>(yuvRange));
                                p.dissolveShader->setUniform("imageChannelsB", imaging::getChannelCount(infoB.pixelType));
                                p.dissolveShader->setUniform("flipBX", infoB.layout.mirror.x);
                                p.dissolveShader->setUniform("flipBY", infoB.layout.mirror.y);
                                bbox2 = imaging::getBBox(layer.imageB->getAspect(), math::BBox2i(0, 0, size.w, size.h));
                                textureRange = math::BBox2f(
                                    .5F - bbox2.w() / static_cast<float>(size.w) / 2.F,
                                    .5F - bbox2.h() / static_cast<float>(size.h) / 2.F,
                                    (bbox2.w() - 1) / static_cast<float>(size.w - 1),
                                    (bbox2.h() - 1) / static_cast<float>(size.h - 1));
                                p.dissolveShader->setUniform("textureRangeBU", math::Vector2f(textureRange.min.x, textureRange.max.x));
                                p.dissolveShader->setUniform("textureRangeBV", math::Vector2f(textureRange.min.y, textureRange.max.y));
                                p.dissolveShader->setUniform("textureSamplerB0", 3);
                                p.dissolveShader->setUniform("textureSamplerB1", 4);
                                p.dissolveShader->setUniform("textureSamplerB2", 5);

                                auto textures = p.textureCache.get(info);
                                copyTextures(layer.image, textures);
                                auto texturesB = p.textureCache.get(infoB, 3);
                                copyTextures(layer.imageB, texturesB, 3);

                                std::vector<uint8_t> vboData;
                                vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
                                VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                                vboP[0].vx = 0;
                                vboP[0].vy = 0;
                                vboP[0].tx = 0;
                                vboP[0].ty = 65535;
                                vboP[1].vx = size.w;
                                vboP[1].vy = 0;
                                vboP[1].tx = 65535;
                                vboP[1].ty = 65535;
                                vboP[2].vx = 0;
                                vboP[2].vy = size.h;
                                vboP[2].tx = 0;
                                vboP[2].ty = 0;
                                vboP[3].vx = size.w;
                                vboP[3].vy = size.h;
                                vboP[3].tx = 65535;
                                vboP[3].ty = 0;
                                auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                                vbo->copy(vboData);

                                auto vao = VAO::create(vbo->getType(), vbo->getID());
                                vao->bind();
                                vao->draw(GL_TRIANGLE_STRIP, 0, 4);

                                p.textureCache.add(info, textures);
                                p.textureCache.add(infoB, texturesB);
                            }
                            else if (layer.image)
                            {
                                p.imageShader->bind();
                                p.imageShader->setUniform("transform.mvp", mvp);

                                drawImage(
                                    layer.image,
                                    imaging::getBBox(layer.image->getAspect(), math::BBox2i(0, 0, size.w, size.h)),
                                    imaging::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                    imageOptions);
                            }
                            else if (layer.imageB)
                            {
                                p.imageShader->bind();
                                p.imageShader->setUniform("transform.mvp", mvp);

                                drawImage(
                                    layer.imageB,
                                    imaging::getBBox(layer.imageB->getAspect(), math::BBox2i(0, 0, size.w, size.h)),
                                    imaging::Color4f(1.F, 1.F, 1.F, layer.transitionValue),
                                    imageOptions);
                            }
                            break;
                        }
                    default:
                        if (layer.image)
                        {
                            p.imageShader->bind();
                            p.imageShader->setUniform("transform.mvp", mvp);

                            drawImage(
                                layer.image,
                                imaging::getBBox(layer.image->getAspect(), math::BBox2i(0, 0, size.w, size.h)),
                                imaging::Color4f(1.F, 1.F, 1.F),
                                imageOptions);
                        }
                        break;
                    }
                }
            }

            if (p.buffer)
            {
                glViewport(0, 0, p.size.w, p.size.h);

                glBlendFunc(GL_ONE, GL_ZERO);

                p.displayShader->bind();
                p.displayShader->setUniform("textureSampler", 0);
                p.displayShader->setUniform("channels", static_cast<int>(displayOptions.channels));
                p.displayShader->setUniform("mirrorX", displayOptions.mirror.x);
                p.displayShader->setUniform("mirrorY", displayOptions.mirror.y);
                const bool colorMatrixEnabled = displayOptions.colorEnabled && displayOptions.color != timeline::Color();
                p.displayShader->setUniform("colorEnabled", colorMatrixEnabled);
                p.displayShader->setUniform("colorAdd", displayOptions.color.add);
                if (colorMatrixEnabled)
                {
                    p.displayShader->setUniform("colorMatrix", timeline::color(displayOptions.color));
                }
                p.displayShader->setUniform("colorInvert", displayOptions.colorEnabled ? displayOptions.color.invert : false);
                p.displayShader->setUniform("levelsEnabled", displayOptions.levelsEnabled);
                p.displayShader->setUniform("levels.inLow", displayOptions.levels.inLow);
                p.displayShader->setUniform("levels.inHigh", displayOptions.levels.inHigh);
                p.displayShader->setUniform("levels.gamma", displayOptions.levels.gamma > 0.F ? (1.F / displayOptions.levels.gamma) : 1000000.F);
                p.displayShader->setUniform("levels.outLow", displayOptions.levels.outLow);
                p.displayShader->setUniform("levels.outHigh", displayOptions.levels.outHigh);
                p.displayShader->setUniform("exposureEnabled", displayOptions.exposureEnabled);
                if (displayOptions.exposureEnabled)
                {
                    const float v = powf(2.F, displayOptions.exposure.exposure + 2.47393F);
                    const float d = displayOptions.exposure.defog;
                    const float k = powf(2.F, displayOptions.exposure.kneeLow);
                    const float f = knee2(
                        powf(2.F, displayOptions.exposure.kneeHigh) - k,
                        powf(2.F, 3.5F) - k);
                    p.displayShader->setUniform("exposure.v", v);
                    p.displayShader->setUniform("exposure.d", d);
                    p.displayShader->setUniform("exposure.k", k);
                    p.displayShader->setUniform("exposure.f", f);
                }
                p.displayShader->setUniform("softClip", displayOptions.softClipEnabled ? displayOptions.softClip : 0.F);

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                std::vector<uint8_t> vboData;
                vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
                VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                vboP[0].vx = bbox.min.x;
                vboP[0].vy = bbox.min.y;
                vboP[0].tx = 0;
                vboP[0].ty = 65535;
                vboP[1].vx = bbox.max.x + 1;
                vboP[1].vy = bbox.min.y;
                vboP[1].tx = 65535;
                vboP[1].ty = 65535;
                vboP[2].vx = bbox.min.x;
                vboP[2].vy = bbox.max.y + 1;
                vboP[2].tx = 0;
                vboP[2].ty = 0;
                vboP[3].vx = bbox.max.x + 1;
                vboP[3].vy = bbox.max.y + 1;
                vboP[3].tx = 65535;
                vboP[3].ty = 0;
                auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                vbo->copy(vboData);

                auto vao = VAO::create(vbo->getType(), vbo->getID());
                vao->bind();
                vao->draw(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
    }
}
