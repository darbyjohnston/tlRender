// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrGL/Render.h>

#include <tlrGL/Mesh.h>
#include <tlrGL/OffscreenBuffer.h>
#include <tlrGL/Shader.h>
#include <tlrGL/Texture.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Color.h>

#include <array>

namespace tlr
{
    namespace gl
    {
        namespace
        {
            struct VBOVertex
            {
                float    vx;
                float    vy;
                uint16_t tx;
                uint16_t ty;
            };

            enum class ColorMode
            {
                Solid,
                Texture,
                TextureAlpha
            };
        }

        void Render::_init()
        {
            _shader = Shader::create(
                "#version 410\n"
                "\n"
                "in vec3 aPos;\n"
                "in vec2 aTexture;\n"
                "\n"
                "out vec2 texture;\n"
                "\n"
                "uniform struct Transform\n"
                "{\n"
                "    mat4 mvp;\n"
                "} transform;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = transform.mvp * vec4(aPos, 1.0);\n"
                "    texture = aTexture;\n"
                "}\n",
                "#version 410\n"
                "\n"
                "in vec2 texture;\n"
                "out vec4 fragColor;\n"
                "\n"
                "// ColorMode\n"
                "#define COLOR_MODE_SOLID 0\n"
                "#define COLOR_MODE_TEXTURE 1\n"
                "#define COLOR_MODE_TEXTURE_ALPHA 2\n"
                "uniform int colorMode;\n"
                "\n"
                "uniform vec4 color;\n"
                "\n"
                "// tlr::imaging::PixelType\n"
                "#define PIXEL_TYPE_NONE     0\n"
                "#define PIXEL_TYPE_L_U8     1\n"
                "#define PIXEL_TYPE_RGB_U8   2\n"
                "#define PIXEL_TYPE_RGBA_U8  3\n"
                "#define PIXEL_TYPE_RGBA_F16 4\n"
                "#define PIXEL_TYPE_YUV_420P 5\n"
                "uniform int pixelType;\n"
                "uniform sampler2D textureSampler0;\n"
                "uniform sampler2D textureSampler1;\n"
                "uniform sampler2D textureSampler2;\n"
                "\n"
                "vec4 sampleTexture()\n"
                "{\n"
                "    vec4 c;\n"
                "    if (PIXEL_TYPE_YUV_420P == pixelType)\n"
                "    {\n"
                "        float y = texture2D(textureSampler0, texture).r;\n"
                "        float u = texture2D(textureSampler1, texture).r - 0.5;\n"
                "        float v = texture2D(textureSampler2, texture).r - 0.5;\n"
                "        c.r = y + 1.402 * v;\n"
                "        c.g = y - 0.344 * u - 0.714 * v;\n"
                "        c.b = y + 1.772 * u;\n"
                "        c.a = 1.0;\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        c = texture2D(textureSampler0, texture);\n"
                "    }\n"
                "    return c;\n"
                "}\n"
                "\n"
                "void main()\n"
                "{\n"
                "    if (COLOR_MODE_SOLID == colorMode)\n"
                "    {\n"
                "        fragColor = color;\n"
                "    }\n"
                "    else if (COLOR_MODE_TEXTURE == colorMode)\n"
                "    {\n"
                "        vec4 t = sampleTexture();\n"
                "        fragColor = t * color;\n"
                "    }\n"
                "    else if (COLOR_MODE_TEXTURE_ALPHA == colorMode)\n"
                "    {\n"
                "        vec4 t = sampleTexture();\n"
                "        fragColor.r = color.r;\n"
                "        fragColor.g = color.g;\n"
                "        fragColor.b = color.b;\n"
                "        fragColor.a = t.r;\n"
                "    }\n"
                "}\n");
        }

        Render::Render()
        {}

        Render::~Render()
        {}

        std::shared_ptr<Render> Render::create()
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init();
            return out;
        }

        GLuint Render::getID() const
        {
            return _offscreenBuffer->getID();
        }

        void Render::begin(const imaging::Info& info)
        {
            if (!_offscreenBuffer ||
                (_offscreenBuffer && _offscreenBuffer->getSize() != info.size) ||
                (_offscreenBuffer && _offscreenBuffer->getColorType() != info.pixelType))
            {
                _offscreenBuffer = OffscreenBuffer::create(info.size, info.pixelType);
            }
            _offscreenBuffer->bind();

            glViewport(0, 0, info.size.w, info.size.h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            _shader->bind();
            const auto viewMatrix = math::ortho(
                0.F,
                static_cast<float>(info.size.w),
                static_cast<float>(info.size.h),
                0.F,
                -1.F,
                1.F);
            _shader->setUniform("transform.mvp", viewMatrix);
        }

        void Render::end()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void Render::drawRect(const math::BBox2f& bbox, const imaging::Color4f& color)
        {
            _shader->setUniform("colorMode", static_cast<int>(ColorMode::Solid));
            _shader->setUniform("color", color);

            std::vector<uint8_t> vboData;
            vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
            VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
            vboP[0].vx = bbox.min.x;
            vboP[0].vy = bbox.min.y;
            vboP[0].tx = 0;
            vboP[0].ty = 0;
            vboP[1].vx = bbox.max.x;
            vboP[1].vy = bbox.min.y;
            vboP[1].tx = 0;
            vboP[1].ty = 0;
            vboP[2].vx = bbox.min.x;
            vboP[2].vy = bbox.max.y;
            vboP[2].tx = 0;
            vboP[2].ty = 0;
            vboP[3].vx = bbox.max.x;
            vboP[3].vy = bbox.max.y;
            vboP[3].tx = 0;
            vboP[3].ty = 0;
            auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
            vbo->copy(vboData);

            auto vao = VAO::create(vbo->getType(), vbo->getID());
            vao->bind();
            vao->draw(GL_TRIANGLE_STRIP, 0, 4);
        }

        void Render::drawImage(const std::shared_ptr<imaging::Image>& image, const math::BBox2f& bbox)
        {
            const auto& info = image->getInfo();
            _shader->setUniform("colorMode", static_cast<int>(ColorMode::Texture));
            _shader->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F));
            _shader->setUniform("pixelType", static_cast<int>(info.pixelType));
            _shader->setUniform("textureSampler0", 0);
            _shader->setUniform("textureSampler1", 1);
            _shader->setUniform("textureSampler2", 2);

            //! \todo Cache textures for reuse.
            std::vector<std::shared_ptr<Texture> > textures;
            switch (info.pixelType)
            {
            case imaging::PixelType::L_U8:
            case imaging::PixelType::RGB_U8:
            case imaging::PixelType::RGBA_U8:
            case imaging::PixelType::RGBA_F16:
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                auto texture = Texture::create(info);
                texture->copy(*image);
                textures.push_back(texture);
                break;
            }
            case imaging::PixelType::YUV_420P:
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U8);
                auto texture = Texture::create(infoTmp);
                texture->copy(image->getData(), infoTmp);
                textures.push_back(texture);

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE1));
                const std::size_t w = info.size.w;
                const std::size_t h = info.size.h;
                const std::size_t w2 = w / 2;
                const std::size_t h2 = h / 2;
                infoTmp = imaging::Info(imaging::Size(w2, h2), imaging::PixelType::L_U8);
                texture = Texture::create(infoTmp);
                texture->copy(image->getData() + (w * h), infoTmp);
                textures.push_back(texture);

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE2));
                texture = Texture::create(infoTmp);
                texture->copy(image->getData() + (w * h) + (w2 * h2), infoTmp);
                textures.push_back(texture);
                break;
            }
            default:
                break;
            }

            std::vector<uint8_t> vboData;
            vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
            VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
            vboP[0].vx = bbox.min.x;
            vboP[0].vy = bbox.min.y;
            vboP[0].tx = 0;
            vboP[0].ty = 0;
            vboP[1].vx = bbox.max.x;
            vboP[1].vy = bbox.min.y;
            vboP[1].tx = 65535;
            vboP[1].ty = 0;
            vboP[2].vx = bbox.min.x;
            vboP[2].vy = bbox.max.y;
            vboP[2].tx = 0;
            vboP[2].ty = 65535;
            vboP[3].vx = bbox.max.x;
            vboP[3].vy = bbox.max.y;
            vboP[3].tx = 65535;
            vboP[3].ty = 65535;
            auto vbo = VBO::create(4, VBOType::Pos2_F32_UV_U16);
            vbo->copy(vboData);

            auto vao = VAO::create(vbo->getType(), vbo->getID());
            vao->bind();
            vao->draw(GL_TRIANGLE_STRIP, 0, 4);
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<Glyph> >& glyphs,
            const math::Vector2f& pos,
            const imaging::Color4f& color)
        {
            _shader->setUniform("colorMode", static_cast<int>(ColorMode::TextureAlpha));
            _shader->setUniform("color", color);
            _shader->setUniform("pixelType", static_cast<int>(imaging::PixelType::L_U8));
            _shader->setUniform("textureSampler0", 0);

            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));

            float x = 0.F;
            int32_t rsbDeltaPrev = 0;
            uint8_t textureIndex = 0;
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (rsbDeltaPrev - glyph->lsbDelta > 32)
                    {
                        x -= 1.F;
                    }
                    else if (rsbDeltaPrev - glyph->lsbDelta < -31)
                    {
                        x += 1.F;
                    }
                    rsbDeltaPrev = glyph->rsbDelta;

                    if (glyph->image && glyph->image->isValid())
                    {
                        std::shared_ptr<Texture> texture;
                        if (!_glyphTextureCache.get(glyph->glyphInfo, texture))
                        {
                            texture = Texture::create(glyph->image->getInfo());
                            texture->copy(*glyph->image);
                            _glyphTextureCache.add(glyph->glyphInfo, texture);
                        }
                        glBindTexture(GL_TEXTURE_2D, texture->getID());

                        const imaging::Size& size = glyph->image->getSize();
                        const math::Vector2f& offset = glyph->offset;
                        const math::BBox2f bbox(pos.x + x + offset.x, pos.y - offset.y, size.w, size.h);

                        std::vector<uint8_t> vboData;
                        vboData.resize(4 * getByteCount(VBOType::Pos2_F32_UV_U16));
                        VBOVertex* vboP = reinterpret_cast<VBOVertex*>(vboData.data());
                        vboP[0].vx = bbox.min.x;
                        vboP[0].vy = bbox.min.y;
                        vboP[0].tx = 0;
                        vboP[0].ty = 0;
                        vboP[1].vx = bbox.max.x;
                        vboP[1].vy = bbox.min.y;
                        vboP[1].tx = 65535;
                        vboP[1].ty = 0;
                        vboP[2].vx = bbox.min.x;
                        vboP[2].vy = bbox.max.y;
                        vboP[2].tx = 0;
                        vboP[2].ty = 65535;
                        vboP[3].vx = bbox.max.x;
                        vboP[3].vy = bbox.max.y;
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
