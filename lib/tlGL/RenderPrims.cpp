// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGL/Mesh.h>

namespace tl
{
    namespace gl
    {
        namespace
        {
            void copyTextures(
                const std::shared_ptr<imaging::Image>& image,
                const std::vector<std::shared_ptr<Texture> >& textures,
                size_t offset = 0)
            {
                std::vector<std::shared_ptr<Texture> > out;
                const auto& info = image->getInfo();
                switch (info.pixelType)
                {
                case imaging::PixelType::YUV_420P:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->copy(image->getData(), textures[0]->getInfo());

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(image->getData() + (w * h), textures[1]->getInfo());

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->copy(image->getData() + (w * h) + (w2 * h2), textures[2]->getInfo());
                    break;
                }
                default:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->copy(*image);
                    break;
                }
                }
            }
        }

        void Render::drawRect(
            const math::BBox2i& bbox,
            const imaging::Color4f& color)
        {
            TLRENDER_P();

            p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Solid));
            p.shader->setUniform("color", color);

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

        namespace
        {
            void swap(uint16_t& a, uint16_t& b)
            {
                uint16_t tmp = a;
                a = b;
                b = tmp;
            }

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

        void Render::drawImage(
            const std::shared_ptr<imaging::Image>& image,
            const math::BBox2i& bbox,
            const imaging::Color4f& color,
            const timeline::ImageOptions& imageOptions)
        {
            TLRENDER_P();

            const auto& info = image->getInfo();
            p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Image));
            p.shader->setUniform("color", color);
            p.shader->setUniform("pixelType", static_cast<int>(info.pixelType));
            imaging::YUVRange yuvRange = info.yuvRange;
            switch (imageOptions.yuvRange)
            {
            case timeline::YUVRange::Full:  yuvRange = imaging::YUVRange::Full;  break;
            case timeline::YUVRange::Video: yuvRange = imaging::YUVRange::Video; break;
            default: break;
            }
            p.shader->setUniform("yuvRange", static_cast<int>(yuvRange));
            p.shader->setUniform("imageChannels", imaging::getChannelCount(info.pixelType));
            p.shader->setUniform("textureSampler0", 0);
            p.shader->setUniform("textureSampler1", 1);
            p.shader->setUniform("textureSampler2", 2);
            const bool colorMatrixEnabled = imageOptions.colorEnabled && imageOptions.color != timeline::Color();
            p.shader->setUniform("colorEnabled", colorMatrixEnabled);
            p.shader->setUniform("colorAdd", imageOptions.color.add);
            if (colorMatrixEnabled)
            {
                p.shader->setUniform("colorMatrix", timeline::color(imageOptions.color));
            }
            p.shader->setUniform("colorInvert", imageOptions.colorEnabled ? imageOptions.color.invert : false);
            p.shader->setUniform("levelsEnabled", imageOptions.levelsEnabled);
            p.shader->setUniform("levels.inLow", imageOptions.levels.inLow);
            p.shader->setUniform("levels.inHigh", imageOptions.levels.inHigh);
            p.shader->setUniform("levels.gamma", imageOptions.levels.gamma > 0.F ? (1.F / imageOptions.levels.gamma) : 1000000.F);
            p.shader->setUniform("levels.outLow", imageOptions.levels.outLow);
            p.shader->setUniform("levels.outHigh", imageOptions.levels.outHigh);
            p.shader->setUniform("exposureEnabled", imageOptions.exposureEnabled);
            if (imageOptions.exposureEnabled)
            {
                const float v = powf(2.F, imageOptions.exposure.exposure + 2.47393F);
                const float d = imageOptions.exposure.defog;
                const float k = powf(2.F, imageOptions.exposure.kneeLow);
                const float f = knee2(
                    powf(2.F, imageOptions.exposure.kneeHigh) - k,
                    powf(2.F, 3.5F) - k);
                p.shader->setUniform("exposure.v", v);
                p.shader->setUniform("exposure.d", d);
                p.shader->setUniform("exposure.k", k);
                p.shader->setUniform("exposure.f", f);
            }
            p.shader->setUniform("softClip", imageOptions.softClipEnabled ? imageOptions.softClip : 0.F);
            p.shader->setUniform("channels", static_cast<int>(imageOptions.channels));

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
            if (info.layout.mirror.x)
            {
                swap(vboP[0].tx, vboP[1].tx);
                swap(vboP[2].tx, vboP[3].tx);
            }
            if (info.layout.mirror.y)
            {
                swap(vboP[0].ty, vboP[2].ty);
                swap(vboP[1].ty, vboP[3].ty);
            }
            if (imageOptions.mirror.x)
            {
                swap(vboP[0].tx, vboP[1].tx);
                swap(vboP[2].tx, vboP[3].tx);
            }
            if (imageOptions.mirror.y)
            {
                swap(vboP[0].ty, vboP[2].ty);
                swap(vboP[1].ty, vboP[3].ty);
            }
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

            p.shader->setUniform("drawMode", static_cast<int>(DrawMode::TextureAlpha));
            p.shader->setUniform("color", color);
            p.shader->setUniform("pixelType", static_cast<int>(imaging::PixelType::L_U8));
            p.shader->setUniform("textureSampler0", 0);

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
    }
}
