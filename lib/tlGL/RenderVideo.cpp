// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGL/Mesh.h>
#include <tlGL/State.h>
#include <tlGL/Util.h>

#include <tlCore/Math.h>

#include <tlGlad/gl.h>

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
                        !bboxes.empty() ? bboxes[0] : math::BBox2i(0, 0, p.size.w, p.size.h),
                        !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                        !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                }
                break;
            case timeline::CompareMode::B:
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
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
                        math::BBox2i(0, 0, p.size.w, p.size.h),
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
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                        displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                }
                break;
            }
            case timeline::CompareMode::Overlay:
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
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
                        SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);

                        _drawVideo(
                            videoData[0],
                            math::BBox2i(0, 0, p.size.w, p.size.h),
                            !imageOptions.empty() ? std::make_shared<timeline::ImageOptions>(imageOptions[0]) : nullptr,
                            !displayOptions.empty() ? displayOptions[0] : timeline::DisplayOptions());
                    }

                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    p.shaders["texture"]->bind();
                    p.shaders["texture"]->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F, compareOptions.overlay));
                    p.shaders["texture"]->setUniform("textureSampler", 0);

                    if (p.buffers["overlay"])
                    {
                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                        glBindTexture(GL_TEXTURE_2D, p.buffers["overlay"]->getColorID());
                    }

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
                break;
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
                        SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);
                        _drawVideo(
                            videoData[0],
                            math::BBox2i(0, 0, p.size.w, p.size.h),
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
                            SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);
                            glClearColor(0.F, 0.F, 0.F, 0.F);
                            glClear(GL_COLOR_BUFFER_BIT);
                            _drawVideo(
                                videoData[1],
                                math::BBox2i(0, 0, p.size.w, p.size.h),
                                imageOptions.size() > 1 ? std::make_shared<timeline::ImageOptions>(imageOptions[1]) : nullptr,
                                displayOptions.size() > 1 ? displayOptions[1] : timeline::DisplayOptions());
                        }
                    }

                    p.shaders["difference"]->bind();
                    p.shaders["difference"]->setUniform("textureSampler", 0);
                    p.shaders["difference"]->setUniform("textureSamplerB", 1);

                    if (p.buffers["difference0"])
                    {
                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                        glBindTexture(GL_TEXTURE_2D, p.buffers["difference0"]->getColorID());
                    }
                    if (p.buffers["difference1"])
                    {
                        glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                        glBindTexture(GL_TEXTURE_2D, p.buffers["difference1"]->getColorID());
                    }

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

            imaging::Size size;
            if (!videoData.layers.empty() &&
                videoData.layers[0].image)
            {
                const auto& imageSize = videoData.layers[0].image->getSize();
                size.w = imageSize.w * imageSize.pixelAspectRatio;
                size.h = imageSize.h;
            }
            OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
            if (imageOptions.get())
            {
                offscreenBufferOptions.colorFilters = displayOptions.imageFilters;
            }
            if (doCreate(p.buffers["video"], size, offscreenBufferOptions))
            {
                p.buffers["video"] = OffscreenBuffer::create(size, offscreenBufferOptions);
            }

            if (p.buffers["video"])
            {
                OffscreenBufferBinding binding(p.buffers["video"]);
                SetAndRestore scissorTest(GL_SCISSOR_TEST, GL_FALSE);
                glViewport(0, 0, size.w, size.h);
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);

                math::Matrix4x4f viewPrevious = p.view;
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
                                const auto& info = layer.image->getInfo();
                                auto textures = p.textureCache.get(info, displayOptions.imageFilters);
                                copyTextures(layer.image, textures);
                                const auto& infoB = layer.imageB->getInfo();
                                auto texturesB = p.textureCache.get(infoB, displayOptions.imageFilters, 3);
                                copyTextures(layer.imageB, texturesB, 3);

                                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                                p.shaders["dissolve"]->bind();
                                p.shaders["dissolve"]->setUniform("transform.mvp", mvp);
                                p.shaders["dissolve"]->setUniform("transition", layer.transitionValue);

                                p.shaders["dissolve"]->setUniform("pixelType", static_cast<int>(layer.image->getPixelType()));
                                imaging::VideoLevels videoLevels = info.videoLevels;
                                switch (imageOptions.get() ? imageOptions->videoLevels : layer.imageOptions.videoLevels)
                                {
                                case timeline::InputVideoLevels::FullRange:  videoLevels = imaging::VideoLevels::FullRange;  break;
                                case timeline::InputVideoLevels::LegalRange: videoLevels = imaging::VideoLevels::LegalRange; break;
                                default: break;
                                }
                                p.shaders["dissolve"]->setUniform("videoLevels", static_cast<int>(videoLevels));
                                p.shaders["dissolve"]->setUniform("yuvCoefficients", imaging::getYUVCoefficients(info.yuvCoefficients));
                                p.shaders["dissolve"]->setUniform("imageChannels", imaging::getChannelCount(info.pixelType));
                                p.shaders["dissolve"]->setUniform("mirrorX", info.layout.mirror.x);
                                p.shaders["dissolve"]->setUniform("mirrorY", info.layout.mirror.y);
                                math::BBox2i bboxA = imaging::getBBox(layer.image->getAspect(), math::BBox2i(0, 0, size.w, size.h));
                                math::BBox2f textureRangeA(
                                    .5F - bboxA.w() / static_cast<float>(size.w) / 2.F,
                                    .5F - bboxA.h() / static_cast<float>(size.h) / 2.F,
                                    (bboxA.w() - 1) / static_cast<float>(size.w - 1),
                                    (bboxA.h() - 1) / static_cast<float>(size.h - 1));
                                p.shaders["dissolve"]->setUniform("textureRangeU", math::Vector2f(textureRangeA.min.x, textureRangeA.max.x));
                                p.shaders["dissolve"]->setUniform("textureRangeV", math::Vector2f(textureRangeA.min.y, textureRangeA.max.y));
                                p.shaders["dissolve"]->setUniform("textureSampler0", 0);
                                p.shaders["dissolve"]->setUniform("textureSampler1", 1);
                                p.shaders["dissolve"]->setUniform("textureSampler2", 2);

                                p.shaders["dissolve"]->setUniform("pixelTypeB", static_cast<int>(layer.imageB->getPixelType()));
                                videoLevels = infoB.videoLevels;
                                switch (imageOptions.get() ? imageOptions->videoLevels : layer.imageOptionsB.videoLevels)
                                {
                                case timeline::InputVideoLevels::FullRange:  videoLevels = imaging::VideoLevels::FullRange;  break;
                                case timeline::InputVideoLevels::LegalRange: videoLevels = imaging::VideoLevels::LegalRange; break;
                                default: break;
                                }
                                p.shaders["dissolve"]->setUniform("videoLevelsB", static_cast<int>(videoLevels));
                                p.shaders["dissolve"]->setUniform("yuvCoefficientsB", imaging::getYUVCoefficients(infoB.yuvCoefficients));
                                p.shaders["dissolve"]->setUniform("imageChannelsB", imaging::getChannelCount(infoB.pixelType));
                                p.shaders["dissolve"]->setUniform("mirrorBX", infoB.layout.mirror.x);
                                p.shaders["dissolve"]->setUniform("mirrorBY", infoB.layout.mirror.y);
                                math::BBox2i bboxB = imaging::getBBox(layer.imageB->getAspect(), math::BBox2i(0, 0, size.w, size.h));
                                math::BBox2f textureRangeB = math::BBox2f(
                                    .5F - bboxB.w() / static_cast<float>(size.w) / 2.F,
                                    .5F - bboxB.h() / static_cast<float>(size.h) / 2.F,
                                    (bboxB.w() - 1) / static_cast<float>(size.w - 1),
                                    (bboxB.h() - 1) / static_cast<float>(size.h - 1));
                                p.shaders["dissolve"]->setUniform("textureRangeBU", math::Vector2f(textureRangeB.min.x, textureRangeB.max.x));
                                p.shaders["dissolve"]->setUniform("textureRangeBV", math::Vector2f(textureRangeB.min.y, textureRangeB.max.y));
                                p.shaders["dissolve"]->setUniform("textureSamplerB0", 3);
                                p.shaders["dissolve"]->setUniform("textureSamplerB1", 4);
                                p.shaders["dissolve"]->setUniform("textureSamplerB2", 5);

                                if (p.vbos["video"])
                                {
                                    p.vbos["video"]->copy(convert(
                                        geom::bbox(math::BBox2i(0, 0, size.w, size.h)),
                                        p.vbos["video"]->getType()));
                                }
                                if (p.vaos["video"])
                                {
                                    p.vaos["video"]->bind();
                                    p.vaos["video"]->draw(GL_TRIANGLES, 0, p.vbos["video"]->getSize());
                                }

                                p.textureCache.add(info, displayOptions.imageFilters, textures);
                                p.textureCache.add(infoB, displayOptions.imageFilters, texturesB);
                            }
                            else if (layer.image)
                            {
                                p.shaders["image"]->bind();
                                p.shaders["image"]->setUniform("transform.mvp", mvp);

                                drawImage(
                                    layer.image,
                                    imaging::getBBox(layer.image->getAspect(), math::BBox2i(0, 0, size.w, size.h)),
                                    imaging::Color4f(1.F, 1.F, 1.F, 1.F - layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptions);
                            }
                            else if (layer.imageB)
                            {
                                p.shaders["image"]->bind();
                                p.shaders["image"]->setUniform("transform.mvp", mvp);

                                drawImage(
                                    layer.imageB,
                                    imaging::getBBox(layer.imageB->getAspect(), math::BBox2i(0, 0, size.w, size.h)),
                                    imaging::Color4f(1.F, 1.F, 1.F, layer.transitionValue),
                                    imageOptions.get() ? *imageOptions : layer.imageOptionsB);
                            }
                            break;
                        }
                    default:
                        if (layer.image)
                        {
                            p.shaders["image"]->bind();
                            p.shaders["image"]->setUniform("transform.mvp", mvp);

                            drawImage(
                                layer.image,
                                imaging::getBBox(layer.image->getAspect(), math::BBox2i(0, 0, size.w, size.h)),
                                imaging::Color4f(1.F, 1.F, 1.F),
                                imageOptions.get() ? *imageOptions : layer.imageOptions);
                        }
                        break;
                    }
                }

                p.shaders["dissolve"]->setUniform("transform.mvp", viewPrevious);
                p.shaders["image"]->setUniform("transform.mvp", viewPrevious);
            }

            if (p.buffers["video"])
            {
                glViewport(0, 0, p.size.w, p.size.h);

                glBlendFunc(GL_ONE, GL_ZERO);

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
                    p.vbos["video"]->copy(convert(geom::bbox(bbox), p.vbos["video"]->getType()));
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
