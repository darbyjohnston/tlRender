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
        void Render::drawRect(
            const math::BBox2i& bbox,
            const imaging::Color4f& color)
        {
            TLRENDER_P();

            p.rectShader->bind();
            p.rectShader->setUniform("color", color);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            std::vector<uint8_t> vboData;
            vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
            VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
            vboP[0].vx = bbox.min.x;
            vboP[0].vy = bbox.min.y;
            vboP[0].tx = 0;
            vboP[0].ty = 0;
            vboP[1].vx = bbox.max.x + 1;
            vboP[1].vy = bbox.min.y;
            vboP[1].tx = 0;
            vboP[1].ty = 0;
            vboP[2].vx = bbox.min.x;
            vboP[2].vy = bbox.max.y + 1;
            vboP[2].tx = 0;
            vboP[2].ty = 0;
            vboP[3].vx = bbox.max.x + 1;
            vboP[3].vy = bbox.max.y + 1;
            vboP[3].tx = 0;
            vboP[3].ty = 0;
            auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
            vbo->copy(vboData);

            auto vao = VAO::create(vbo->getType(), vbo->getID());
            vao->bind();
            vao->draw(GL_TRIANGLE_STRIP, 0, 4);
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
            const math::Vector2i& pos,
            const imaging::Color4f& color)
        {
            TLRENDER_P();

            p.textShader->bind();
            p.textShader->setUniform("color", color);
            p.textShader->setUniform("textureSampler", 0);

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

                        std::vector<uint8_t> vboData;
                        vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
                        VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                        vboP[0].vx = bbox.min.x;
                        vboP[0].vy = bbox.min.y;
                        vboP[0].tx = 0;
                        vboP[0].ty = 0;
                        vboP[1].vx = bbox.max.x + 1;
                        vboP[1].vy = bbox.min.y;
                        vboP[1].tx = 65535;
                        vboP[1].ty = 0;
                        vboP[2].vx = bbox.min.x;
                        vboP[2].vy = bbox.max.y + 1;
                        vboP[2].tx = 0;
                        vboP[2].ty = 65535;
                        vboP[3].vx = bbox.max.x + 1;
                        vboP[3].vy = bbox.max.y + 1;
                        vboP[3].tx = 65535;
                        vboP[3].ty = 65535;
                        auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
                        vbo->copy(vboData);

                        auto vao = VAO::create(vbo->getType(), vbo->getID());
                        vao->bind();
                        vao->draw(GL_TRIANGLE_STRIP, 0, 4);
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

            p.imageShader->bind();
            p.imageShader->setUniform("color", color);
            const auto& info = image->getInfo();
            p.imageShader->setUniform("pixelType", static_cast<int>(info.pixelType));
            imaging::YUVRange yuvRange = info.yuvRange;
            switch (imageOptions.yuvRange)
            {
            case timeline::YUVRange::Full:  yuvRange = imaging::YUVRange::Full;  break;
            case timeline::YUVRange::Video: yuvRange = imaging::YUVRange::Video; break;
            default: break;
            }
            p.imageShader->setUniform("yuvRange", static_cast<int>(yuvRange));
            p.imageShader->setUniform("imageChannels", imaging::getChannelCount(info.pixelType));
            p.imageShader->setUniform("flipX", info.layout.mirror.x);
            p.imageShader->setUniform("flipY", info.layout.mirror.y);
            p.imageShader->setUniform("textureSampler0", 0);
            p.imageShader->setUniform("textureSampler1", 1);
            p.imageShader->setUniform("textureSampler2", 2);

            switch (imageOptions.alphaBlend)
            {
            case timeline::AlphaBlend::None:
                glBlendFunc(GL_ONE, GL_ZERO);
                break;
            case timeline::AlphaBlend::Straight:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case timeline::AlphaBlend::Premultiplied:
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                break;
            default: break;
            }

            auto textures = p.textureCache.get(info);
            copyTextures(image, textures);

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

            p.textureCache.add(info, textures);
        }
    }
}
