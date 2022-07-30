// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

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

            p.meshShader->bind();
            p.meshShader->setUniform("color", color);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            std::vector<uint8_t> vboData;
            vboData.resize(4 * getByteCount(VBOType::Pos2_F32));
            Pos2_F32* vboP = reinterpret_cast<Pos2_F32*>(vboData.data());
            vboP[0].vx = bbox.min.x;
            vboP[0].vy = bbox.min.y;
            vboP[1].vx = bbox.max.x + 1;
            vboP[1].vy = bbox.min.y;
            vboP[2].vx = bbox.min.x;
            vboP[2].vy = bbox.max.y + 1;
            vboP[3].vx = bbox.max.x + 1;
            vboP[3].vy = bbox.max.y + 1;
            if (p.rectVBO)
            {
                p.rectVBO->copy(vboData);
            }

            if (p.rectVAO)
            {
                p.rectVAO->bind();
                p.rectVAO->draw(GL_TRIANGLE_STRIP, 0, 4);
            }
        }

        void Render::drawMesh(
            const geom::TriangleMesh2& mesh,
            const imaging::Color4f& color)
        {
            TLRENDER_P();

            const size_t size = mesh.triangles.size();
            if (size > 0)
            {
                p.meshShader->bind();
                p.meshShader->setUniform("color", color);

                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                auto vboData = convert(mesh, VBOType::Pos2_F32, math::SizeTRange(0, size - 1));
                if (!p.meshVBO || (p.meshVBO && p.meshVBO->getSize() != size * 3))
                {
                    p.meshVBO = VBO::create(size * 3, VBOType::Pos2_F32);
                }
                if (p.meshVBO)
                {
                    p.meshVBO->copy(vboData);
                }

                if (!p.meshVAO && p.meshVBO)
                {
                    p.meshVAO = VAO::create(p.meshVBO->getType(), p.meshVBO->getID());
                }
                if (p.meshVAO && p.meshVBO)
                {
                    p.meshVAO->bind();
                    p.meshVAO->draw(GL_TRIANGLES, 0, p.meshVBO->getSize());
                }
            }
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
                        Pos2_F32_UV_U16* vboP = reinterpret_cast<Pos2_F32_UV_U16*>(vboData.data());
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
                        if (p.imageVBO)
                        {
                            p.imageVBO->copy(vboData);
                        }

                        if (p.imageVAO)
                        {
                            p.imageVAO->bind();
                            p.imageVAO->draw(GL_TRIANGLE_STRIP, 0, 4);
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
            auto textures = p.textureCache.get(info);
            copyTextures(image, textures);

            p.imageShader->bind();
            p.imageShader->setUniform("color", color);
            p.imageShader->setUniform("pixelType", static_cast<int>(info.pixelType));
            imaging::YUVRange yuvRange = info.yuvRange;
            switch (imageOptions.yuvRange)
            {
            case timeline::YUVRange::Full:  yuvRange = imaging::YUVRange::Full;  break;
            case timeline::YUVRange::Video: yuvRange = imaging::YUVRange::Video; break;
            default: break;
            }
            p.imageShader->setUniform("yuvRange", static_cast<int>(yuvRange));
            p.imageShader->setUniform("yuvCoefficients", imaging::getYUVCoefficients(info.yuvCoefficients));
            p.imageShader->setUniform("imageChannels", imaging::getChannelCount(info.pixelType));
            p.imageShader->setUniform("flipX", info.layout.mirror.x);
            p.imageShader->setUniform("flipY", info.layout.mirror.y);
            switch (info.pixelType)
            {
            case imaging::PixelType::YUV_420P_U8:
            case imaging::PixelType::YUV_422P_U8:
            case imaging::PixelType::YUV_444P_U8:
            case imaging::PixelType::YUV_420P_U16:
            case imaging::PixelType::YUV_422P_U16:
            case imaging::PixelType::YUV_444P_U16:
                p.imageShader->setUniform("textureSampler1", 1);
                p.imageShader->setUniform("textureSampler2", 2);
            default:
                p.imageShader->setUniform("textureSampler0", 0);
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

            std::vector<uint8_t> vboData;
            vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
            Pos2_F32_UV_U16* vboP = reinterpret_cast<Pos2_F32_UV_U16*>(vboData.data());
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
            if (p.imageVBO)
            {
                p.imageVBO->copy(vboData);
            }

            if (p.imageVAO)
            {
                p.imageVAO->bind();
                p.imageVAO->draw(GL_TRIANGLE_STRIP, 0, 4);
            }

            p.textureCache.add(info, textures);
        }
    }
}
