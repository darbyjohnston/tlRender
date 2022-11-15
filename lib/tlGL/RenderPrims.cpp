// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGlad/gl.h>

#include <glm/gtc/matrix_transform.hpp>

namespace tl
{
    namespace gl
    {
        void Render::drawRect(
            const math::BBox2i& bbox,
            const imaging::Color4f& color)
        {
            TLRENDER_P();

            p.shaders["mesh"]->bind();
            p.shaders["mesh"]->setUniform("color", color);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (p.vbos["rect"])
            {
                p.vbos["rect"]->copy(convert(geom::bbox(bbox), p.vbos["rect"]->getType()));
            }
            if (p.vaos["rect"])
            {
                p.vaos["rect"]->bind();
                p.vaos["rect"]->draw(GL_TRIANGLES, 0, p.vbos["rect"]->getSize());
            }
        }

        void Render::drawMesh(
            const geom::TriangleMesh2& mesh,
            const imaging::Color4f& color,
            const math::Matrix4x4f& mvp)
        {
            TLRENDER_P();

            const size_t size = mesh.triangles.size();
            if (size > 0)
            {
                p.shaders["mesh"]->bind();
                p.shaders["mesh"]->setUniform("color", color);
                p.shaders["mesh"]->setUniform("transform.mvp", mvp );

                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                if (!p.vbos["mesh"] || (p.vbos["mesh"] && p.vbos["mesh"]->getSize() != size * 3))
                {
                    p.vbos["mesh"] = VBO::create(size * 3, VBOType::Pos2_F32);
                    p.vaos["mesh"].reset();
                }
                if (p.vbos["mesh"])
                {
                    p.vbos["mesh"]->copy(convert(mesh, VBOType::Pos2_F32));
                }

                if (!p.vaos["mesh"] && p.vbos["mesh"])
                {
                    p.vaos["mesh"] = VAO::create(p.vbos["mesh"]->getType(), p.vbos["mesh"]->getID());
                }
                if (p.vaos["mesh"] && p.vbos["mesh"])
                {
                    p.vaos["mesh"]->bind();
                    p.vaos["mesh"]->draw(GL_TRIANGLES, 0, p.vbos["mesh"]->getSize());
                }
            }
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
            const math::Vector2i& pos,
            const imaging::Color4f& color)
        {
            TLRENDER_P();

            p.shaders["text"]->bind();
            p.shaders["text"]->setUniform("color", color);
            p.shaders["text"]->setUniform("textureSampler", 0);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));

            int x = 0;
            int32_t rsbDeltaPrev = 0;
            uint8_t textureIndex = 0;
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (rsbDeltaPrev - glyph->lsbDelta > 32)
                    {
                        x -= 1;
                    }
                    else if (rsbDeltaPrev - glyph->lsbDelta < -31)
                    {
                        x += 1;
                    }
                    rsbDeltaPrev = glyph->rsbDelta;

                    if (!glyph->data.empty())
                    {
                        std::shared_ptr<Texture> texture;
                        if (!p.glyphTextureCache.get(glyph->glyphInfo, texture))
                        {
                            const imaging::Info info(glyph->width, glyph->height, imaging::PixelType::L_U8);
                            texture = Texture::create(info);
                            texture->copy(glyph->data.data(), info);
                            p.glyphTextureCache.add(glyph->glyphInfo, texture);
                        }
                        glBindTexture(GL_TEXTURE_2D, texture->getID());

                        const math::Vector2i& offset = glyph->offset;
                        const math::BBox2i bbox(pos.x + x + offset.x, pos.y - offset.y, glyph->width, glyph->height);

                        if (p.vbos["text"])
                        {
                            p.vbos["text"]->copy(convert(geom::bbox(bbox), p.vbos["text"]->getType()));
                        }
                        if (p.vaos["text"])
                        {
                            p.vaos["text"]->bind();
                            p.vaos["text"]->draw(GL_TRIANGLES, 0, p.vbos["text"]->getSize());
                        }
                    }

                    x += glyph->advance;
                }
            }
        }

        void Render::drawImage(
            const std::shared_ptr<imaging::Image>& image,
            const math::BBox2i& bbox,
            const imaging::Color4f& color,
            const timeline::ImageOptions& imageOptions)
        {
            TLRENDER_P();

            const auto& info = image->getInfo();
            auto textures = p.textureCache.get(info, imageOptions.imageFilters);
            copyTextures(image, textures);

            p.shaders["image"]->bind();
            p.shaders["image"]->setUniform("color", color);
            p.shaders["image"]->setUniform("pixelType", static_cast<int>(info.pixelType));
            imaging::VideoLevels videoLevels = info.videoLevels;
            switch (imageOptions.videoLevels)
            {
            case timeline::InputVideoLevels::FullRange:  videoLevels = imaging::VideoLevels::FullRange;  break;
            case timeline::InputVideoLevels::LegalRange: videoLevels = imaging::VideoLevels::LegalRange; break;
            default: break;
            }
            p.shaders["image"]->setUniform("videoLevels", static_cast<int>(videoLevels));
            p.shaders["image"]->setUniform("yuvCoefficients", imaging::getYUVCoefficients(info.yuvCoefficients));
            p.shaders["image"]->setUniform("imageChannels", imaging::getChannelCount(info.pixelType));
            p.shaders["image"]->setUniform("mirrorX", info.layout.mirror.x);
            p.shaders["image"]->setUniform("mirrorY", info.layout.mirror.y);
            switch (info.pixelType)
            {
            case imaging::PixelType::YUV_420P_U8:
            case imaging::PixelType::YUV_422P_U8:
            case imaging::PixelType::YUV_444P_U8:
            case imaging::PixelType::YUV_420P_U16:
            case imaging::PixelType::YUV_422P_U16:
            case imaging::PixelType::YUV_444P_U16:
                p.shaders["image"]->setUniform("textureSampler1", 1);
                p.shaders["image"]->setUniform("textureSampler2", 2);
            default:
                p.shaders["image"]->setUniform("textureSampler0", 0);
                break;
            }

            switch (imageOptions.alphaBlend)
            {
            case timeline::AlphaBlend::None:
                glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
                break;
            case timeline::AlphaBlend::Straight:
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
                break;
            case timeline::AlphaBlend::Premultiplied:
                glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
                break;
            default: break;
            }

            if (p.vbos["image"])
            {
                p.vbos["image"]->copy(convert(geom::bbox(bbox), p.vbos["image"]->getType()));
            }
            if (p.vaos["image"])
            {
                p.vaos["image"]->bind();
                p.vaos["image"]->draw(GL_TRIANGLES, 0, p.vbos["image"]->getSize());
            }

            p.textureCache.add(info, imageOptions.imageFilters, textures);
        }
    }
}
