// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrGL/Render.h>

#include <tlrGL/Mesh.h>
#include <tlrGL/OffscreenBuffer.h>
#include <tlrGL/Shader.h>
#include <tlrGL/Texture.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Context.h>
#include <tlrCore/Error.h>
#include <tlrCore/FontSystem.h>
#include <tlrCore/LRUCache.h>
#include <tlrCore/OCIO.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

#include <OpenColorIO/OpenColorIO.h>

#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <list>

#define _USE_MATH_DEFINES
#include <math.h>

namespace OCIO = OCIO_NAMESPACE;

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

            enum class DrawMode
            {
                Solid,
                TextureAlpha,
                Image
            };

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
                "// enum tlr::imaging::PixelType\n"
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
                "// enum tlr::imaging::YUVRange\n"
                "const uint YUVRange_Full  = 0;\n"
                "const uint YUVRange_Video = 1;\n"
                "\n"
                "// enum tlr::gl::ImageChannelsDisplay\n"
                "const uint ImageChannelsDisplay_Color = 0;\n"
                "const uint ImageChannelsDisplay_Red   = 1;\n"
                "const uint ImageChannelsDisplay_Green = 2;\n"
                "const uint ImageChannelsDisplay_Blue  = 3;\n"
                "const uint ImageChannelsDisplay_Alpha = 4;\n"
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
                "uniform int         imageChannelsDisplay;\n"
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
                "        // Swizzle for the image channels display.\n"
                "        if (ImageChannelsDisplay_Red == imageChannelsDisplay)\n"
                "        {\n"
                "            c.g = c.r;\n"
                "            c.b = c.r;\n"
                "        }\n"
                "        else if (ImageChannelsDisplay_Green == imageChannelsDisplay)\n"
                "        {\n"
                "            c.r = c.g;\n"
                "            c.b = c.g;\n"
                "        }\n"
                "        else if (ImageChannelsDisplay_Blue == imageChannelsDisplay)\n"
                "        {\n"
                "            c.r = c.b;\n"
                "            c.g = c.b;\n"
                "        }\n"
                "        else if (ImageChannelsDisplay_Alpha == imageChannelsDisplay)\n"
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

            class TextureCache
            {
            public:
                void setSize(size_t value)
                {
                    if (value == _size)
                        return;
                    _size = value;
                    _cacheUpdate();
                }

                std::vector<std::shared_ptr<Texture> > get(const imaging::Info& info)
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

            private:
                void _cacheUpdate()
                {
                    while (_cache.size() > _size)
                    {
                        _cache.pop_back();
                    }
                }

                size_t _size = 4;
                std::list<std::pair<imaging::Info, std::vector<std::shared_ptr<Texture> > > > _cache;
            };
        }

        struct Render::Private
        {
            std::weak_ptr<core::Context> context;
            
            imaging::ColorConfig colorConfig;
            OCIO::ConstConfigRcPtr ocioConfig;
            OCIO::DisplayViewTransformRcPtr ocioTransform;
            OCIO::LegacyViewingPipelineRcPtr ocioVP;
            OCIO::ConstProcessorRcPtr ocioProcessor;
            OCIO::ConstGPUProcessorRcPtr ocioGpuProcessor;
            OCIO::GpuShaderDescRcPtr ocioShaderDesc;
            struct TextureId
            {
                TextureId(
                    unsigned    id,
                    std::string name,
                    std::string sampler,
                    unsigned    type);

                unsigned    id = -1;
                std::string name;
                std::string sampler;
                unsigned    type = -1;
            };
            std::vector<TextureId> colorTextures;

            imaging::Size size;

            std::shared_ptr<Shader> shader;

            std::shared_ptr<OffscreenBuffer> offscreenBuffer;

            TextureCache textureCache;

            memory::LRUCache<imaging::GlyphInfo, std::shared_ptr<Texture> > glyphTextureCache;
        };

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

        void Render::_init(const std::shared_ptr<core::Context>& context)
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

        std::shared_ptr<Render> Render::create(const std::shared_ptr<core::Context>& context)
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
            TLR_PRIVATE_P();
            if (config == p.colorConfig)
                return;

            _delColorConfig();

            p.colorConfig = config;

            if (!p.colorConfig.config.empty())
            {
                p.ocioConfig = OCIO::Config::CreateFromFile(p.colorConfig.config.c_str());
            }
            else
            {
                p.ocioConfig = OCIO::GetCurrentConfig();
            }
            if (!p.ocioConfig)
            {
                throw std::runtime_error("Cannot get OCIO configuration");
            }

            const std::string display = !p.colorConfig.display.empty() ?
                p.colorConfig.display :
                p.ocioConfig->getDefaultDisplay();
            const std::string view = !p.colorConfig.view.empty() ?
                p.colorConfig.view :
                p.ocioConfig->getDefaultView(display.c_str());

            p.ocioTransform = OCIO::DisplayViewTransform::Create();
            if (!p.ocioTransform)
            {
                _delColorConfig();
                throw std::runtime_error("Cannot create OCIO transform");
            }
            p.ocioTransform->setSrc(p.colorConfig.input.c_str());
            p.ocioTransform->setDisplay(display.c_str());
            p.ocioTransform->setView(view.c_str());

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

            p.shader.reset();
        }

        void Render::begin(const imaging::Size& size)
        {
            TLR_PRIVATE_P();

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
                    //context->log("tlr::gl::Render", source);
                    context->log("tlr::gl::Render", "Creating fragment shader");
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
            p.shader->setUniform("transform.mvp", viewMatrix);

            for (size_t i = 0; i < p.colorTextures.size(); ++i)
            {
                glActiveTexture(GL_TEXTURE3 + i);
                glBindTexture(p.colorTextures[i].type, p.colorTextures[i].id);
                p.shader->setUniform(p.colorTextures[i].sampler, static_cast<int>(3 + i));
            }
        }

        void Render::end()
        {}

        void Render::drawRect(
            const math::BBox2i& bbox,
            const imaging::Color4f& color)
        {
            TLR_PRIVATE_P();

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

            glm::mat4x4 brightnessMatrix(const glm::vec3& value)
            {
                return glm::mat4x4(
                    value.x, 0.F,     0.F,     0.F,
                    0.F,     value.y, 0.F,     0.F,
                    0.F,     0.F,     value.z, 0.F,
                    0.F,     0.F,     0.F,     1.F);
            }

            glm::mat4x4 contrastMatrix(const glm::vec3& value)
            {
                return
                    glm::mat4x4(
                        1.F, 0.F, 0.F, -.5F,
                        0.F, 1.F, 0.F, -.5F,
                        0.F, 0.F, 1.F, -.5F,
                        0.F, 0.F, 0.F, 1.F) *
                    glm::mat4x4(
                        value.x, 0.F,      0.F,      0.F,
                        0.F,      value.y, 0.F,      0.F,
                        0.F,      0.F,      value.z, 0.F,
                        0.F,      0.F,      0.F,      1.F) *
                    glm::mat4x4(
                        1.F, 0.F, 0.F,  .5F,
                        0.F, 1.F, 0.F,  .5F,
                        0.F, 0.F, 1.F,  .5F,
                        0.F, 0.F, 0.F, 1.F);
            }

            glm::mat4x4 saturationMatrix(const glm::vec3& value)
            {
                const glm::vec3 s(
                    (1.F - value.x) * .3086F,
                    (1.F - value.y) * .6094F,
                    (1.F - value.z) * .0820F);
                return glm::mat4x4(
                    s.x + value.x, s.y,           s.z,           0.F,
                    s.x,           s.y + value.y, s.z,           0.F,
                    s.x,           s.y,           s.z + value.z, 0.F,
                    0.F,           0.F,           0.F,           1.F);
            }

            glm::mat4x4 tintMatrix(float v)
            {
                const float c = cos(v * M_PI * 2.F);
                const float c2 = 1.F - c;
                const float c3 = 1.F / 3.F * c2;
                const float s = sin(v * M_PI * 2.F);
                const float sq = sqrtf(1.F / 3.F);
                return glm::mat4x4(
                    c + c2 / 3.F, c3 - sq * s, c3 + sq * s, 0.F,
                    c3 + sq * s,  c + c3,      c3 - sq * s, 0.F,
                    c3 - sq * s,  c3 + sq * s, c + c3,      0.F,
                    0.F,          0.F,         0.F,         1.F);
            }

            glm::mat4x4 colorMatrix(const render::ImageColor& in)
            {
                return
                    brightnessMatrix(in.brightness) *
                    contrastMatrix(in.contrast) *
                    saturationMatrix(in.saturation) *
                    tintMatrix(in.tint);
            }
        }

        void Render::drawImage(
            const std::shared_ptr<imaging::Image>& image,
            const math::BBox2i& bbox,
            const imaging::Color4f& color,
            const render::ImageOptions& imageOptions)
        {
            TLR_PRIVATE_P();

            const auto& info = image->getInfo();
            p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Image));
            p.shader->setUniform("color", color);
            p.shader->setUniform("pixelType", static_cast<int>(info.pixelType));
            imaging::YUVRange yuvRange = info.yuvRange;
            switch (imageOptions.yuvRange)
            {
            case render::YUVRange::Full:  yuvRange = imaging::YUVRange::Full;  break;
            case render::YUVRange::Video: yuvRange = imaging::YUVRange::Video; break;
            default: break;
            }
            p.shader->setUniform("yuvRange", static_cast<int>(yuvRange));
            p.shader->setUniform("imageChannels", imaging::getChannelCount(info.pixelType));
            p.shader->setUniform("textureSampler0", 0);
            p.shader->setUniform("textureSampler1", 1);
            p.shader->setUniform("textureSampler2", 2);
            const bool colorMatrixEnabled = imageOptions.colorEnabled && imageOptions.color != render::ImageColor();
            p.shader->setUniform("colorEnabled", colorMatrixEnabled);
            p.shader->setUniform("colorAdd", imageOptions.color.add);
            if (colorMatrixEnabled)
            {
                p.shader->setUniform("colorMatrix", colorMatrix(imageOptions.color));
            }
            p.shader->setUniform("colorInvert", imageOptions.color.invert);
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
            p.shader->setUniform("imageChannelsDisplay", static_cast<int>(imageOptions.channelsDisplay));

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

        void Render::drawVideo(
            const std::vector<timeline::VideoData>& videoData,
            const std::vector<render::ImageOptions>& imageOptions,
            const render::CompareOptions& compareOptions)
        {
            TLR_PRIVATE_P();
            switch (compareOptions.mode)
            {
            case render::CompareMode::A:
                if (!videoData.empty())
                {
                    _drawVideo(
                        videoData[0],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        !imageOptions.empty() ? imageOptions[0] : render::ImageOptions());
                }
                break;
            case render::CompareMode::B:
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        imageOptions.size() > 1 ? imageOptions[1] : render::ImageOptions());
                }
                break;
            case render::CompareMode::Wipe:
            {
                const int x = p.size.w * compareOptions.wipe;
                glEnable(GL_SCISSOR_TEST);
                glScissor(0, 0, x, p.size.h);
                if (!videoData.empty())
                {
                    _drawVideo(
                        videoData[0],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        !imageOptions.empty() ? imageOptions[0] : render::ImageOptions());
                }
                glScissor(x, 0, p.size.w - x, p.size.h);
                if (videoData.size() > 1)
                {
                    _drawVideo(
                        videoData[1],
                        math::BBox2i(0, 0, p.size.w, p.size.h),
                        imageOptions.size() > 1 ? imageOptions[1] : render::ImageOptions());
                }
                glDisable(GL_SCISSOR_TEST);
                break;
            }
            case render::CompareMode::Tiles:
            {
                for (size_t i = 0; i < 4 && i < videoData.size(); ++i)
                {
                    const int w = p.size.w / 2;
                    const int h = p.size.h / 2;
                    const int y = i / 2 * h;
                    const int x = i % 2 * w;
                    _drawVideo(
                        videoData[i],
                        math::BBox2i(x, y, w, h).margin(-10),
                        i < imageOptions.size() ? imageOptions[i] : render::ImageOptions());
                }
                break;
            }
            default: break;
            }
        }

        void Render::drawText(
            const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
            const glm::ivec2& pos,
            const imaging::Color4f& color)
        {
            TLR_PRIVATE_P();

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

                        const glm::ivec2& offset = glyph->offset;
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

        void Render::_delColorConfig()
        {
            TLR_PRIVATE_P();
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

        void Render::_drawVideo(
            const timeline::VideoData& videoData,
            const math::BBox2i& bbox,
            const render::ImageOptions& imageOptions)
        {
            TLR_PRIVATE_P();
            for (const auto& layer : videoData.layers)
            {
                switch (layer.transition)
                {
                case timeline::Transition::Dissolve:
                {
                    if (!p.offscreenBuffer || (p.offscreenBuffer && p.offscreenBuffer->getSize() != p.size))
                    {
                        p.offscreenBuffer = OffscreenBuffer::create(p.size, imaging::PixelType::RGBA_F32);
                    }

                    {
                        auto binding = OffscreenBufferBinding(p.offscreenBuffer);
                        glClearColor(0.F, 0.F, 0.F, 0.F);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
                        render::ImageOptions imageOptionsTmp;
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

                    p.shader->setUniform("drawMode", static_cast<int>(DrawMode::Image));
                    p.shader->setUniform("color", imaging::Color4f(1.F, 1.F, 1.F));
                    p.shader->setUniform("pixelType", static_cast<int>(imaging::PixelType::RGBA_F32));
                    p.shader->setUniform("textureSampler0", 0);

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
                    glBindTexture(GL_TEXTURE_2D, p.offscreenBuffer->getColorID());

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
