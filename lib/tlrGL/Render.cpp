// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrGL/Render.h>

#include <tlrGL/Mesh.h>
#include <tlrGL/Shader.h>
#include <tlrGL/Texture.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Color.h>
#include <tlrCore/StringFormat.h>

#include <array>

namespace tlr
{
    namespace gl
    {
        bool ColorConfig::operator == (const ColorConfig& other) const
        {
            return config == other.config &&
                input == other.input &&
                display == other.display &&
                view == other.view;
        }

        bool ColorConfig::operator != (const ColorConfig& other) const
        {
            return !(*this == other);
        }

        Render::TextureId::TextureId(
            unsigned id,
            std::string name,
            std::string sampler,
            unsigned type) :
            id(id),
            name(name),
            sampler(sampler),
            type(type)
        {}

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
                TextureColorConfig,
                TextureAlpha
            };

            const std::string colorFunctionName = "OCIODisplay";

            const std::string colorFunctionNoOp =
                "uniform sampler3D ocio_lut3d_0Sampler;\n"
                "vec4 OCIODisplay(in vec4 inPixel)\n"
                "{\n"
                "    return inPixel;\n"
                "}\n";

            const std::string vertexSource =
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
                "}\n";

            const std::string fragmentSource =
                "#version 410\n"
                "\n"
                "in vec2 texture;\n"
                "out vec4 fragColor;\n"
                "\n"
                "// ColorMode\n"
                "const uint ColorMode_Solid              = 0;\n"
                "const uint ColorMode_Texture            = 1;\n"
                "const uint ColorMode_TextureColorConfig = 2;\n"
                "const uint ColorMode_TextureAlpha       = 3;\n"
                "uniform int colorMode;\n"
                "\n"
                "uniform vec4 color;\n"
                "\n"
                "// tlr::imaging::PixelType\n"
                "const uint PixelType_None     = 0;\n"
                "const uint PixelType_L_U8     = 1;\n"
                "const uint PixelType_L_U16    = 2;\n"
                "const uint PixelType_L_F32    = 3;\n"
                "const uint PixelType_LA_U8    = 4;\n"
                "const uint PixelType_LA_U16   = 5;\n"
                "const uint PixelType_LA_F32   = 6;\n"
                "const uint PixelType_RGB_U8   = 7;\n"
                "const uint PixelType_RGB_U16  = 8;\n"
                "const uint PixelType_RGB_F32  = 9;\n"
                "const uint PixelType_RGBA_U8  = 10;\n"
                "const uint PixelType_RGBA_U16 = 11;\n"
                "const uint PixelType_RGBA_F16 = 12;\n"
                "const uint PixelType_RGBA_F32 = 13;\n"
                "const uint PixelType_YUV_420P = 14;\n"
                "uniform int pixelType;\n"
                "uniform sampler2D textureSampler0;\n"
                "uniform sampler2D textureSampler1;\n"
                "uniform sampler2D textureSampler2;\n"
                "uniform sampler2D textureSampler3;\n"
                "uniform sampler2D textureSampler4;\n"
                "uniform sampler2D textureSampler5;\n"
                "\n"
                "// tlr::timeline::Transition\n"
                "const uint Transition_None     = 0;\n"
                "const uint Transition_Dissolve = 1;\n"
                "uniform int transition;\n"
                "uniform float transitionValue;\n"
                "\n"
                "// $color"
                "\n"
                "vec4 sampleTexture(sampler2D s0, sampler2D s1, sampler2D s2)\n"
                "{\n"
                "    vec4 c;\n"
                "    if (PixelType_YUV_420P == pixelType)\n"
                "    {\n"
                "        float y = texture2D(s0, texture).r;\n"
                "        float u = texture2D(s1, texture).r - 0.5;\n"
                "        float v = texture2D(s2, texture).r - 0.5;\n"
                "        c.r = y + 1.402 * v;\n"
                "        c.g = y - 0.344 * u - 0.714 * v;\n"
                "        c.b = y + 1.772 * u;\n"
                "        c.a = 1.0;\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        c = texture2D(s0, texture);\n"
                "    }\n"
                "    return c;\n"
                "}\n"
                "\n"
                "void main()\n"
                "{\n"
                "    if (ColorMode_Solid == colorMode)\n"
                "    {\n"
                "        fragColor = color;\n"
                "    }\n"
                "    else if (ColorMode_Texture == colorMode)\n"
                "    {\n"
                "        vec4 t = sampleTexture(textureSampler0, textureSampler1, textureSampler2);\n"
                "        if (Transition_None == transition)\n"
                "        {\n"
                "            fragColor = t * color;\n"
                "        }\n"
                "        else if (Transition_Dissolve == transition)\n"
                "        {\n"
                "            vec4 t2 = sampleTexture(textureSampler3, textureSampler4, textureSampler5);\n"
                "            fragColor = ((t * (1.0 - transitionValue)) + (t2 * transitionValue)) * color;\n"
                "        }\n"
                "    }\n"
                "    else if (ColorMode_TextureColorConfig == colorMode)\n"
                "    {\n"
                "        vec4 t = sampleTexture(textureSampler0, textureSampler1, textureSampler2);\n"
                "        if (Transition_None == transition)\n"
                "        {\n"
                "            fragColor = OCIODisplay(t) * color;\n"
                "        }\n"
                "        else if (Transition_Dissolve == transition)\n"
                "        {\n"
                "            vec4 t2 = sampleTexture(textureSampler3, textureSampler4, textureSampler5);\n"
                "            fragColor = OCIODisplay(((t * (1.0 - transitionValue)) + (t2 * transitionValue))) * color;\n"
                "        }\n"
                "    }\n"
                "    else if (ColorMode_TextureAlpha == colorMode)\n"
                "    {\n"
                "        vec4 t = sampleTexture(textureSampler0, textureSampler1, textureSampler2);\n"
                "        fragColor.r = color.r;\n"
                "        fragColor.g = color.g;\n"
                "        fragColor.b = color.b;\n"
                "        fragColor.a = t.r;\n"
                "    }\n"
                "}\n";

            void setTextureParameters(GLenum textureType, OCIO::Interpolation interpolation)
            {
                if (OCIO::INTERP_NEAREST == interpolation)
                {
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                }
                else
                {
                    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }

                glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            }
        }

        void Render::_init()
        {}

        Render::Render()
        {}

        Render::~Render()
        {
            for (size_t i = 0; i < _colorTextures.size(); ++i)
            {
                glDeleteTextures(1, &_colorTextures[i].id);
            }
        }

        std::shared_ptr<Render> Render::create()
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init();
            return out;
        }

        void Render::setColorConfig(const ColorConfig& config)
        {
            if (config == _colorConfig)
                return;

            for (size_t i = 0; i < _colorTextures.size(); ++i)
            {
                glDeleteTextures(1, &_colorTextures[i].id);
            }
            _ocioShaderDesc.reset();
            _ocioGpuProcessor.reset();
            _ocioProcessor.reset();

            _colorConfig = config;

            if (!_colorConfig.config.empty())
            {
                _ocioConfig = OCIO::Config::CreateFromFile(_colorConfig.config.c_str());
            }
            else
            {
                _ocioConfig = OCIO::GetCurrentConfig();
            }
            if (_ocioConfig)
            {
                const std::string display = !_colorConfig.display.empty() ?
                    _colorConfig.display :
                    _ocioConfig->getDefaultDisplay();
                const std::string view = !_colorConfig.view.empty() ?
                    _colorConfig.view :
                    _ocioConfig->getDefaultView(display.c_str());
                _ocioProcessor = _ocioConfig->getProcessor(
                    _colorConfig.input.c_str(),
                    display.c_str(),
                    view.c_str(),
                    OCIO::TRANSFORM_DIR_FORWARD);
                _ocioGpuProcessor = _ocioProcessor->getDefaultGPUProcessor();
                _ocioShaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                _ocioShaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_2);
                _ocioShaderDesc->setFunctionName("OCIODisplay");
                _ocioGpuProcessor->extractGpuShaderInfo(_ocioShaderDesc);

                // Create 3D textures.
                const unsigned num3DTextures = _ocioShaderDesc->getNum3DTextures();
                unsigned currentTexture = 0;
                for (unsigned i = 0; i < num3DTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned edgelen = 0;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    _ocioShaderDesc->get3DTexture(i, textureName, samplerName, edgelen, interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        0 == edgelen)
                    {
                        throw std::runtime_error("The texture data is corrupted");
                    }

                    const float* values = nullptr;
                    _ocioShaderDesc->get3DTextureValues(i, values);
                    if (!values)
                    {
                        throw std::runtime_error("The texture values are missing");
                    }

                    unsigned textureId = 0;
                    glGenTextures(1, &textureId);
                    glActiveTexture(GL_TEXTURE3 + currentTexture);
                    glBindTexture(GL_TEXTURE_3D, textureId);
                    setTextureParameters(GL_TEXTURE_3D, interpolation);
                    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, edgelen, edgelen, edgelen, 0, GL_RGB, GL_FLOAT, values);
                    _colorTextures.push_back(TextureId(textureId, textureName, samplerName, GL_TEXTURE_3D));
                }

                // Create 1D textures.
                const unsigned numTextures = _ocioShaderDesc->getNumTextures();
                for (unsigned i = 0; i < numTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned width = 0;
                    unsigned height = 0;
                    OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    _ocioShaderDesc->getTexture(i, textureName, samplerName, width, height, channel, interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        width == 0)
                    {
                        throw std::runtime_error("The texture data is corrupted");
                    }

                    const float* values = nullptr;
                    _ocioShaderDesc->getTextureValues(i, values);
                    if (!values)
                    {
                        throw std::runtime_error("The texture values are missing");
                    }

                    unsigned textureId = 0;
                    GLint internalformat = GL_RGB32F;
                    GLenum format = GL_RGB;
                    if (OCIO::GpuShaderCreator::TEXTURE_RED_CHANNEL == channel)
                    {
                        internalformat = GL_R32F;
                        format = GL_RED;
                    }
                    glGenTextures(1, &textureId);
                    glActiveTexture(GL_TEXTURE3 + currentTexture);
                    if (height > 1)
                    {
                        glBindTexture(GL_TEXTURE_2D, textureId);
                        setTextureParameters(GL_TEXTURE_2D, interpolation);
                        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_FLOAT, values);
                    }
                    else
                    {
                        glBindTexture(GL_TEXTURE_1D, textureId);
                        setTextureParameters(GL_TEXTURE_1D, interpolation);
                        glTexImage1D(GL_TEXTURE_1D, 0, internalformat, width, 0, format, GL_FLOAT, values);
                    }
                    _colorTextures.push_back(TextureId(textureId, textureName, samplerName, (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }

            _shader.reset();
        }

        void Render::begin(const imaging::Size& size, bool flipY)
        {
            _size = size;

            glViewport(0, 0, _size.w, _size.h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (!_shader)
            {
                std::string source = fragmentSource;
                const std::string token = "// $color";
                const auto i = source.find(token);
                if (i != std::string::npos)
                {
                    source.replace(i, token.size(), _ocioShaderDesc ? _ocioShaderDesc->getShaderText() : colorFunctionNoOp);
                }
                _shader = Shader::create(vertexSource, source);
            }
            _shader->bind();
            const auto viewMatrix = math::ortho(
                0.F,
                static_cast<float>(_size.w),
                flipY ? 0.F : static_cast<float>(_size.h),
                flipY ? static_cast<float>(_size.h) : 0.F,
                -1.F,
                1.F);
            _shader->setUniform("transform.mvp", viewMatrix);

            for (size_t i = 0; i < _colorTextures.size(); ++i)
            {
                glActiveTexture(GL_TEXTURE3 + i);
                glBindTexture(_colorTextures[i].type, _colorTextures[i].id);
                _shader->setUniform(_colorTextures[i].sampler, static_cast<int>(3 + i));
            }
        }

        void Render::end()
        {}

        void Render::drawRect(
            const math::BBox2f& bbox,
            const imaging::Color4f& color)
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

        namespace
        {
            std::vector<std::shared_ptr<Texture> > getTextures(const std::shared_ptr<imaging::Image>& image, size_t offset = 0)
            {
                std::vector<std::shared_ptr<Texture> > out;
                const auto& info = image->getInfo();
                switch (info.pixelType)
                {
                case imaging::PixelType::YUV_420P:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U8);
                    auto texture = Texture::create(infoTmp);
                    texture->copy(image->getData(), infoTmp);
                    out.push_back(texture);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    infoTmp = imaging::Info(imaging::Size(w2, h2), imaging::PixelType::L_U8);
                    texture = Texture::create(infoTmp);
                    texture->copy(image->getData() + (w * h), infoTmp);
                    out.push_back(texture);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    texture = Texture::create(infoTmp);
                    texture->copy(image->getData() + (w * h) + (w2 * h2), infoTmp);
                    out.push_back(texture);
                    break;
                }
                default:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto texture = Texture::create(info);
                    texture->copy(*image);
                    out.push_back(texture);
                    break;
                }
                }
                return out;
            }
        }

        void Render::drawImage(
            const std::shared_ptr<imaging::Image>& image,
            const math::BBox2f& bbox)
        {
            const auto& info = image->getInfo();
            _shader->setUniform("colorMode", static_cast<int>(ColorMode::TextureColorConfig));
            _shader->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F));
            _shader->setUniform("pixelType", static_cast<int>(info.pixelType));
            _shader->setUniform("textureSampler0", 0);
            _shader->setUniform("textureSampler1", 1);
            _shader->setUniform("textureSampler2", 2);
            _shader->setUniform("transition", static_cast<int>(timeline::Transition::None));

            //! \todo Cache textures for reuse.
            auto textures = getTextures(image);

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

        void Render::drawImage(
            const std::shared_ptr<imaging::Image>& image,
            const std::shared_ptr<imaging::Image>& imageB,
            const math::BBox2f& bbox,
            timeline::Transition transition,
            float transitionValue)
        {
            const auto& info = image->getInfo();
            _shader->setUniform("colorMode", static_cast<int>(ColorMode::TextureColorConfig));
            _shader->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F));
            _shader->setUniform("pixelType", static_cast<int>(info.pixelType));
            _shader->setUniform("textureSampler0", 0);
            _shader->setUniform("textureSampler1", 1);
            _shader->setUniform("textureSampler2", 2);
            _shader->setUniform("textureSampler3", 3);
            _shader->setUniform("textureSampler4", 4);
            _shader->setUniform("textureSampler5", 5);
            _shader->setUniform("transition", static_cast<int>(transition));
            _shader->setUniform("transitionValue", transitionValue);

            //! \todo Cache textures for reuse.
            auto textures = getTextures(image);
            auto texturesB = getTextures(imageB, 3);

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

        void Render::drawFrame(const timeline::Frame& frame)
        {
            for (const auto& i : frame.layers)
            {
                if (i.image && i.imageB)
                {
                    drawImage(
                        i.image,
                        i.imageB,
                        imaging::getBBox(i.image->getAspect(), _size),
                        i.transition,
                        i.transitionValue);
                }
                else if (i.image)
                {
                    drawImage(
                        i.image,
                        imaging::getBBox(i.image->getAspect(), _size));
                }
            }
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
