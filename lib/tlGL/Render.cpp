// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGL/Mesh.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <list>

#define _USE_MATH_DEFINES
#include <math.h>

namespace tl
{
    namespace gl
    {
        namespace
        {
            const std::string colorFunctionName = "OCIODisplay";

            const std::string colorFunctionNoOp =
                "vec4 OCIODisplay(vec4 inPixel)\n"
                "{\n"
                "    return inPixel;\n"
                "}\n";

            const std::string vertexSource =
                "#version 410\n"
                "\n"
                "// Inputs\n"
                "in vec3 vPos;\n"
                "in vec2 vTexture;\n"
                "\n"
                "// Outputs\n"
                "out vec2 fTexture;\n"
                "\n"
                "// Uniforms\n"
                "uniform struct Transform\n"
                "{\n"
                "    mat4 mvp;\n"
                "} transform;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                "    fTexture = vTexture;\n"
                "}\n";

            const std::string fragmentSource =
                "#version 410\n"
                "\n"
                "// Inputs\n"
                "in vec2 fTexture;\n"
                "\n"
                "// Outputs\n"
                "out vec4 fColor;\n"
                "\n"
                "// enum DrawMode\n"
                "const uint DrawMode_Solid        = 0;\n"
                "const uint DrawMode_TextureAlpha = 1;\n"
                "const uint DrawMode_Image        = 2;\n"
                "\n"
                "// enum tl::imaging::PixelType\n"
                "const uint PixelType_None     = 0;\n"
                "const uint PixelType_L_U8     = 1;\n"
                "const uint PixelType_L_U16    = 2;\n"
                "const uint PixelType_L_U32    = 3;\n"
                "const uint PixelType_L_F16    = 4;\n"
                "const uint PixelType_L_F32    = 5;\n"
                "const uint PixelType_LA_U8    = 6;\n"
                "const uint PixelType_LA_U32   = 7;\n"
                "const uint PixelType_LA_U16   = 8;\n"
                "const uint PixelType_LA_F16   = 9;\n"
                "const uint PixelType_LA_F32   = 10;\n"
                "const uint PixelType_RGB_U8   = 11;\n"
                "const uint PixelType_RGB_U10  = 12;\n"
                "const uint PixelType_RGB_U16  = 13;\n"
                "const uint PixelType_RGB_U32  = 14;\n"
                "const uint PixelType_RGB_F16  = 15;\n"
                "const uint PixelType_RGB_F32  = 16;\n"
                "const uint PixelType_RGBA_U8  = 17;\n"
                "const uint PixelType_RGBA_U16 = 18;\n"
                "const uint PixelType_RGBA_U32 = 19;\n"
                "const uint PixelType_RGBA_F16 = 20;\n"
                "const uint PixelType_RGBA_F32 = 21;\n"
                "const uint PixelType_YUV_420P = 22;\n"
                "\n"
                "// enum tl::render::YUVRange\n"
                "const uint YUVRange_Full  = 0;\n"
                "const uint YUVRange_Video = 1;\n"
                "\n"
                "// enum tl::render::Channels\n"
                "const uint Channels_Color = 0;\n"
                "const uint Channels_Red   = 1;\n"
                "const uint Channels_Green = 2;\n"
                "const uint Channels_Blue  = 3;\n"
                "const uint Channels_Alpha = 4;\n"
                "\n"
                "struct Levels\n"
                "{\n"
                "    float inLow;\n"
                "    float inHigh;\n"
                "    float gamma;\n"
                "    float outLow;\n"
                "    float outHigh;\n"
                "};\n"
                "\n"
                "struct Exposure\n"
                "{\n"
                "    float v;\n"
                "    float d;\n"
                "    float k;\n"
                "    float f;\n"
                "    float g;\n"
                "};\n"
                "\n"
                "// Uniforms\n"
                "uniform int         drawMode;\n"
                "uniform vec4        color;\n"
                "\n"
                "uniform int         pixelType;\n"
                "uniform int         yuvRange;\n"
                "uniform int         imageChannels;\n"
                "uniform sampler2D   textureSampler0;\n"
                "uniform sampler2D   textureSampler1;\n"
                "uniform sampler2D   textureSampler2;\n"
                "\n"
                "uniform bool        colorEnabled;\n"
                "uniform vec3        colorAdd;\n"
                "uniform mat4        colorMatrix;\n"
                "uniform bool        colorInvert;\n"
                "uniform bool        levelsEnabled;\n"
                "uniform Levels      levels;\n"
                "uniform bool        exposureEnabled;\n"
                "uniform Exposure    exposure;\n"
                "uniform float       softClip;\n"
                "uniform int         channels;\n"
                "\n"
                "vec4 colorFunc(vec4 value, vec3 add, mat4 m)\n"
                "{\n"
                "    vec4 tmp;\n"
                "    tmp[0] = value[0] + add[0];\n"
                "    tmp[1] = value[1] + add[1];\n"
                "    tmp[2] = value[2] + add[2];\n"
                "    tmp[3] = 1.0;\n"
                "    tmp *= m;\n"
                "    tmp[3] = value[3];\n"
                "    return tmp;\n"
                "}\n"
                "\n"
                "vec4 levelsFunc(vec4 value, Levels data)\n"
                "{\n"
                "    vec4 tmp;\n"
                "    tmp[0] = (value[0] - data.inLow) / data.inHigh;\n"
                "    tmp[1] = (value[1] - data.inLow) / data.inHigh;\n"
                "    tmp[2] = (value[2] - data.inLow) / data.inHigh;\n"
                "    if (tmp[0] >= 0.0)\n"
                "        tmp[0] = pow(tmp[0], data.gamma);\n"
                "    if (tmp[1] >= 0.0)\n"
                "        tmp[1] = pow(tmp[1], data.gamma);\n"
                "    if (tmp[2] >= 0.0)\n"
                "        tmp[2] = pow(tmp[2], data.gamma);\n"
                "    value[0] = tmp[0] * data.outHigh + data.outLow;\n"
                "    value[1] = tmp[1] * data.outHigh + data.outLow;\n"
                "    value[2] = tmp[2] * data.outHigh + data.outLow;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "vec4 softClipFunc(vec4 value, float softClip)\n"
                "{\n"
                "    float tmp = 1.0 - softClip;\n"
                "    if (value[0] > tmp)\n"
                "        value[0] = tmp + (1.0 - exp(-(value[0] - tmp) / softClip)) * softClip;\n"
                "    if (value[1] > tmp)\n"
                "        value[1] = tmp + (1.0 - exp(-(value[1] - tmp) / softClip)) * softClip;\n"
                "    if (value[2] > tmp)\n"
                "        value[2] = tmp + (1.0 - exp(-(value[2] - tmp) / softClip)) * softClip;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "float knee(float value, float f)\n"
                "{\n"
                "    return log(value * f + 1.0) / f;\n"
                "}\n"
                "\n"
                "vec4 exposureFunc(vec4 value, Exposure data)\n"
                "{\n"
                "    value[0] = max(0.0, value[0] - data.d) * data.v;\n"
                "    value[1] = max(0.0, value[1] - data.d) * data.v;\n"
                "    value[2] = max(0.0, value[2] - data.d) * data.v;\n"
                "    if (value[0] > data.k)\n"
                "        value[0] = data.k + knee(value[0] - data.k, data.f);\n"
                "    if (value[1] > data.k)\n"
                "        value[1] = data.k + knee(value[1] - data.k, data.f);\n"
                "    if (value[2] > data.k)\n"
                "        value[2] = data.k + knee(value[2] - data.k, data.f);\n"
                "    value[0] *= 0.332;\n"
                "    value[1] *= 0.332;\n"
                "    value[2] *= 0.332;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "// $color"
                "\n"
                "vec4 sampleTexture(sampler2D s0, sampler2D s1, sampler2D s2)\n"
                "{\n"
                "    vec4 c;\n"
                "    if (PixelType_YUV_420P == pixelType)\n"
                "    {\n"
                "        if (YUVRange_Full == yuvRange)\n"
                "        {\n"
                "            float y  = texture(s0, fTexture).r;\n"
                "            float cb = texture(s1, fTexture).r - 128.0 / 255.0;\n"
                "            float cr = texture(s2, fTexture).r - 128.0 / 255.0;\n"
                "            c.r = y + ( 0.0   * cb) + ( 1.4   * cr);\n"
                "            c.g = y + (-0.343 * cb) + (-0.711 * cr);\n"
                "            c.b = y + ( 1.765 * cb) + ( 0.0   * cr);\n"
                "        }\n"
                "        else if (YUVRange_Video == yuvRange)\n"
                "        {\n"
                "            float y  = texture(s0, fTexture).r -  16.0 / 255.0;\n"
                "            float cb = texture(s1, fTexture).r - 128.0 / 255.0;\n"
                "            float cr = texture(s2, fTexture).r - 128.0 / 255.0;\n"
                "            c.r = (1.164 * y) + ( 0.0   * cb) + ( 1.793 * cr);\n"
                "            c.g = (1.164 * y) + (-0.213 * cb) + (-0.533 * cr);\n"
                "            c.b = (1.164 * y) + ( 2.112 * cb) + ( 0.0   * cr);\n"
                "        }\n"
                "        c.a = 1.0;\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        c = texture(s0, fTexture);\n"
                "    }\n"
                "    return c;\n"
                "}\n"
                "\n"
                "void main()\n"
                "{\n"
                "    if (DrawMode_Solid == drawMode)\n"
                "    {\n"
                "        fColor = color;\n"
                "    }\n"
                "    else if (DrawMode_TextureAlpha == drawMode)\n"
                "    {\n"
                "        vec4 c = sampleTexture(textureSampler0, textureSampler1, textureSampler2);\n"
                "        fColor.r = color.r;\n"
                "        fColor.g = color.g;\n"
                "        fColor.b = color.b;\n"
                "        fColor.a = c.r;\n"
                "    }\n"
                "    else if (DrawMode_Image == drawMode)\n"
                "    {\n"
                "        vec4 c = sampleTexture(textureSampler0, textureSampler1, textureSampler2);\n"
                "\n"
                "        // Swizzle for the image channels.\n"
                "        if (1 == imageChannels)\n"
                "        {\n"
                "            c.g = c.b = c.r;\n"
                "            c.a = 1.0;\n"
                "        }\n"
                "        else if (2 == imageChannels)\n"
                "        {\n"
                "            c.a = c.g;\n"
                "            c.g = c.b = c.r;\n"
                "        }\n"
                "        else if (3 == imageChannels)\n"
                "        {\n"
                "            c.a = 1.0;\n"
                "        }\n"
                "\n"
                "        // Apply color transformations.\n"
                "        if (colorEnabled)\n"
                "        {\n"
                "            c = colorFunc(c, colorAdd, colorMatrix);\n"
                "        }\n"
                "        if (colorInvert)\n"
                "        {\n"
                "            c.r = 1.0 - c.r;\n"
                "            c.g = 1.0 - c.g;\n"
                "            c.b = 1.0 - c.b;\n"
                "        }\n"
                "        if (levelsEnabled)\n"
                "        {\n"
                "            c = levelsFunc(c, levels);\n"
                "        }\n"
                "        if (exposureEnabled)\n"
                "        {\n"
                "            c = exposureFunc(c, exposure);\n"
                "        }\n"
                "        if (softClip > 0.0)\n"
                "        {\n"
                "            c = softClipFunc(c, softClip);\n"
                "        }\n"
                "\n"
                "        // Apply color management.\n"
                "        c = OCIODisplay(c);\n"
                "\n"
                "        // Swizzle for the channels display.\n"
                "        if (Channels_Red == channels)\n"
                "        {\n"
                "            c.g = c.r;\n"
                "            c.b = c.r;\n"
                "        }\n"
                "        else if (Channels_Green == channels)\n"
                "        {\n"
                "            c.r = c.g;\n"
                "            c.b = c.g;\n"
                "        }\n"
                "        else if (Channels_Blue == channels)\n"
                "        {\n"
                "            c.r = c.b;\n"
                "            c.g = c.b;\n"
                "        }\n"
                "        else if (Channels_Alpha == channels)\n"
                "        {\n"
                "            c.r = c.a;\n"
                "            c.g = c.a;\n"
                "            c.b = c.a;\n"
                "        }\n"
                "\n"
                "        fColor = c * color;\n"
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

            std::vector<std::shared_ptr<Texture> > getTextures(const imaging::Info& info, size_t offset = 0)
            {
                std::vector<std::shared_ptr<Texture> > out;
                switch (info.pixelType)
                {
                case imaging::PixelType::YUV_420P:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    infoTmp = imaging::Info(imaging::Size(w2, h2), imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    out.push_back(Texture::create(infoTmp));
                    break;
                }
                default:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto texture = Texture::create(info);
                    out.push_back(texture);
                    break;
                }
                }
                return out;
            }
        }

        void TextureCache::setSize(size_t value)
        {
            if (value == _size)
                return;
            _size = value;
            _cacheUpdate();
        }

        std::vector<std::shared_ptr<Texture> > TextureCache::get(const imaging::Info& info)
        {
            std::vector<std::shared_ptr<Texture> > out;
            const auto i = std::find_if(_cache.begin(), _cache.end(),
                [info](const std::pair<imaging::Info, std::vector<std::shared_ptr<Texture> > >& value)
                {
                    return info == value.first;
                });
            if (i != _cache.end())
            {
                out = i->second;
            }
            else
            {
                out = getTextures(info);
                _cache.push_front(std::make_pair(info, out));
                _cacheUpdate();
            }
            return out;
        }

        void TextureCache::_cacheUpdate()
        {
            while (_cache.size() > _size)
            {
                _cache.pop_back();
            }
        }

        Render::Private::TextureId::TextureId(
            unsigned id,
            std::string name,
            std::string sampler,
            unsigned type) :
            id(id),
            name(name),
            sampler(sampler),
            type(type)
        {}

        void Render::_init(const std::shared_ptr<system::Context>& context)
        {
            IRender::_init(context);
        }

        Render::Render() :
            _p(new Private)
        {}

        Render::~Render()
        {
            _delColorConfig();
        }

        std::shared_ptr<Render> Render::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init(context);
            return out;
        }
        
        void Render::setTextureCacheSize(size_t value)
        {
            _p->textureCache.setSize(value);
        }

        void Render::setColorConfig(const imaging::ColorConfig& config)
        {
            TLRENDER_P();
            if (config == p.colorConfig)
                return;

            _delColorConfig();

            p.colorConfig = config;

            if (!p.colorConfig.input.empty() &&
                !p.colorConfig.display.empty() &&
                !p.colorConfig.view.empty())
            {
                if (!p.colorConfig.fileName.empty())
                {
                    p.ocioConfig = OCIO::Config::CreateFromFile(p.colorConfig.fileName.c_str());
                }
                else
                {
                    p.ocioConfig = OCIO::GetCurrentConfig();
                }
                if (!p.ocioConfig)
                {
                    throw std::runtime_error("Cannot get OCIO configuration");
                }

                p.ocioTransform = OCIO::DisplayViewTransform::Create();
                if (!p.ocioTransform)
                {
                    _delColorConfig();
                    throw std::runtime_error("Cannot create OCIO transform");
                }
                p.ocioTransform->setSrc(p.colorConfig.input.c_str());
                p.ocioTransform->setDisplay(p.colorConfig.display.c_str());
                p.ocioTransform->setView(p.colorConfig.view.c_str());

                p.ocioVP = OCIO::LegacyViewingPipeline::Create();
                if (!p.ocioVP)
                {
                    _delColorConfig();
                    throw std::runtime_error("Cannot create OCIO viewing pipeline");
                }
                p.ocioVP->setDisplayViewTransform(p.ocioTransform);
                p.ocioVP->setLooksOverrideEnabled(true);
                p.ocioVP->setLooksOverride(p.colorConfig.look.c_str());

                p.ocioProcessor = p.ocioVP->getProcessor(p.ocioConfig, p.ocioConfig->getCurrentContext());
                if (!p.ocioProcessor)
                {
                    _delColorConfig();
                    throw std::runtime_error("Cannot get OCIO processor");
                }
                p.ocioGpuProcessor = p.ocioProcessor->getDefaultGPUProcessor();
                if (!p.ocioGpuProcessor)
                {
                    _delColorConfig();
                    throw std::runtime_error("Cannot get OCIO GPU processor");
                }
                p.ocioShaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                if (!p.ocioShaderDesc)
                {
                    _delColorConfig();
                    throw std::runtime_error("Cannot create OCIO shader description");
                }
                p.ocioShaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
                p.ocioShaderDesc->setFunctionName(colorFunctionName.c_str());
                p.ocioGpuProcessor->extractGpuShaderInfo(p.ocioShaderDesc);

                // Create 3D textures.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                const unsigned num3DTextures = p.ocioShaderDesc->getNum3DTextures();
                unsigned currentTexture = 0;
                for (unsigned i = 0; i < num3DTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned edgelen = 0;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.ocioShaderDesc->get3DTexture(i, textureName, samplerName, edgelen, interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        0 == edgelen)
                    {
                        _delColorConfig();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.ocioShaderDesc->get3DTextureValues(i, values);
                    if (!values)
                    {
                        _delColorConfig();
                        throw std::runtime_error("The OCIO texture values are missing");
                    }

                    unsigned textureId = 0;
                    glGenTextures(1, &textureId);
                    glActiveTexture(GL_TEXTURE3 + currentTexture);
                    glBindTexture(GL_TEXTURE_3D, textureId);
                    setTextureParameters(GL_TEXTURE_3D, interpolation);
                    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, edgelen, edgelen, edgelen, 0, GL_RGB, GL_FLOAT, values);
                    p.colorTextures.push_back(Private::TextureId(textureId, textureName, samplerName, GL_TEXTURE_3D));
                }

                // Create 1D textures.
                const unsigned numTextures = p.ocioShaderDesc->getNumTextures();
                for (unsigned i = 0; i < numTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned width = 0;
                    unsigned height = 0;
                    OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.ocioShaderDesc->getTexture(i, textureName, samplerName, width, height, channel, interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        width == 0)
                    {
                        _delColorConfig();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.ocioShaderDesc->getTextureValues(i, values);
                    if (!values)
                    {
                        _delColorConfig();
                        throw std::runtime_error("The OCIO texture values are missing");
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
                    p.colorTextures.push_back(Private::TextureId(textureId, textureName, samplerName, (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }

            p.shader.reset();
        }

        void Render::begin(const imaging::Size& size)
        {
            TLRENDER_P();

            p.size = size;

            glViewport(0, 0, p.size.w, p.size.h);
            glClearColor(0.F, 0.F, 0.F, 1.F);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (!p.shader)
            {
                std::string source = fragmentSource;
                const std::string token = "// $color";
                const auto i = source.find(token);
                if (i != std::string::npos)
                {
                    source.replace(i, token.size(), p.ocioShaderDesc ? p.ocioShaderDesc->getShaderText() : colorFunctionNoOp);
                }
                if (auto context = p.context.lock())
                {
                    //context->log("tl::gl::Render", source);
                    context->log("tl::gl::Render", "Creating fragment shader");
                }
                p.shader = Shader::create(vertexSource, source);
            }
            p.shader->bind();
            const auto viewMatrix = glm::ortho(
                0.F,
                static_cast<float>(p.size.w),
                static_cast<float>(p.size.h),
                0.F,
                -1.F,
                1.F);
            p.shader->setUniform(
                "transform.mvp",
                math::Matrix4x4f(
                    viewMatrix[0][0], viewMatrix[0][1], viewMatrix[0][2], viewMatrix[0][3],
                    viewMatrix[1][0], viewMatrix[1][1], viewMatrix[1][2], viewMatrix[1][3],
                    viewMatrix[2][0], viewMatrix[2][1], viewMatrix[2][2], viewMatrix[2][3],
                    viewMatrix[3][0], viewMatrix[3][1], viewMatrix[3][2], viewMatrix[3][3]));

            for (size_t i = 0; i < p.colorTextures.size(); ++i)
            {
                glActiveTexture(GL_TEXTURE3 + i);
                glBindTexture(p.colorTextures[i].type, p.colorTextures[i].id);
                p.shader->setUniform(p.colorTextures[i].sampler, static_cast<int>(3 + i));
            }
        }

        void Render::end()
        {}

        void Render::_delColorConfig()
        {
            TLRENDER_P();
            for (size_t i = 0; i < p.colorTextures.size(); ++i)
            {
                glDeleteTextures(1, &p.colorTextures[i].id);
            }
            p.colorTextures.clear();
            p.ocioShaderDesc.reset();
            p.ocioGpuProcessor.reset();
            p.ocioProcessor.reset();
            p.ocioVP.reset();
            p.ocioTransform.reset();
            p.ocioConfig.reset();
        }
    }
}
