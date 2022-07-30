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
        void copyTextures(
            const std::shared_ptr<imaging::Image>& image,
            const std::vector<std::shared_ptr<Texture> >& textures,
            size_t offset)
        {
            std::vector<std::shared_ptr<Texture> > out;
            const auto& info = image->getInfo();
            switch (info.pixelType)
            {
            case imaging::PixelType::YUV_420P_U8:
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
            case imaging::PixelType::YUV_422P_U8:
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                textures[0]->copy(image->getData(), textures[0]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                const std::size_t w = info.size.w;
                const std::size_t h = info.size.h;
                const std::size_t w2 = w / 2;
                textures[1]->copy(image->getData() + (w * h), textures[1]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                textures[2]->copy(image->getData() + (w * h) + (w2 * h), textures[2]->getInfo());
                break;
            }
            case imaging::PixelType::YUV_444P_U8:
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                textures[0]->copy(image->getData(), textures[0]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                const std::size_t w = info.size.w;
                const std::size_t h = info.size.h;
                textures[1]->copy(image->getData() + (w * h), textures[1]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                textures[2]->copy(image->getData() + (w * h) + (w * h), textures[2]->getInfo());
                break;
            }
            case imaging::PixelType::YUV_420P_U16:
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                textures[0]->copy(image->getData(), textures[0]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                const std::size_t w = info.size.w;
                const std::size_t h = info.size.h;
                const std::size_t w2 = w / 2;
                const std::size_t h2 = h / 2;
                textures[1]->copy(image->getData() + (w * h) * 2, textures[1]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                textures[2]->copy(image->getData() + (w * h) * 2 + (w2 * h2) * 2, textures[2]->getInfo());
                break;
            }
            case imaging::PixelType::YUV_422P_U16:
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                textures[0]->copy(image->getData(), textures[0]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                const std::size_t w = info.size.w;
                const std::size_t h = info.size.h;
                const std::size_t w2 = w / 2;
                textures[1]->copy(image->getData() + (w * h) * 2, textures[1]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                textures[2]->copy(image->getData() + (w * h) * 2 + (w2 * h) * 2, textures[2]->getInfo());
                break;
            }
            case imaging::PixelType::YUV_444P_U16:
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                textures[0]->copy(image->getData(), textures[0]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                const std::size_t w = info.size.w;
                const std::size_t h = info.size.h;
                textures[1]->copy(image->getData() + (w * h) * 2, textures[1]->getInfo());

                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                textures[2]->copy(image->getData() + (w * h) * 2 + (w * h) * 2, textures[2]->getInfo());
                break;
            }
            default:
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                textures[0]->copy(*image);
                break;
            }
        }

        namespace
        {
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

            std::vector<std::shared_ptr<Texture> > getTextures(const imaging::Info& info, size_t offset)
            {
                std::vector<std::shared_ptr<Texture> > out;
                TextureOptions options;
                //options.pbo = true;
                switch (info.pixelType)
                {
                case imaging::PixelType::YUV_420P_U8:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    infoTmp = imaging::Info(imaging::Size(w2, h2), imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    out.push_back(Texture::create(infoTmp, options));
                    break;
                }
                case imaging::PixelType::YUV_422P_U8:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    infoTmp = imaging::Info(imaging::Size(w2, h), imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    out.push_back(Texture::create(infoTmp, options));
                    break;
                }
                case imaging::PixelType::YUV_444P_U8:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    infoTmp = imaging::Info(imaging::Size(w, h), imaging::PixelType::L_U8);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    out.push_back(Texture::create(infoTmp, options));
                    break;
                }
                case imaging::PixelType::YUV_420P_U16:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U16);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    infoTmp = imaging::Info(imaging::Size(w2, h2), imaging::PixelType::L_U16);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    out.push_back(Texture::create(infoTmp, options));
                    break;
                }
                case imaging::PixelType::YUV_422P_U16:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U16);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    infoTmp = imaging::Info(imaging::Size(w2, h), imaging::PixelType::L_U16);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    out.push_back(Texture::create(infoTmp, options));
                    break;
                }
                case imaging::PixelType::YUV_444P_U16:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto infoTmp = imaging::Info(info.size, imaging::PixelType::L_U16);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    infoTmp = imaging::Info(imaging::Size(w, h), imaging::PixelType::L_U16);
                    out.push_back(Texture::create(infoTmp, options));

                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    out.push_back(Texture::create(infoTmp, options));
                    break;
                }
                default:
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    auto texture = Texture::create(info, options);
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

        std::vector<std::shared_ptr<Texture> > TextureCache::get(const imaging::Info& info, size_t offset)
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
                _cache.erase(i);
            }
            else
            {
                out = getTextures(info, offset);
            }
            return out;
        }

        void TextureCache::add(const imaging::Info& info, const std::vector<std::shared_ptr<Texture> >& textures)
        {
            _cache.push_front(std::make_pair(info, textures));
            _cacheUpdate();
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
                p.ocioShaderDesc->setFunctionName(colorFunctionName().c_str());
                p.ocioGpuProcessor->extractGpuShaderInfo(p.ocioShaderDesc);

                // Create 3D textures.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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
                    if (!textureName  ||
                        !*textureName ||
                        !samplerName  ||
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
                    if (!textureName  ||
                        !*textureName ||
                        !samplerName  ||
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
                    p.colorTextures.push_back(Private::TextureId(
                        textureId,
                        textureName,
                        samplerName,
                        (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }

            p.displayShader.reset();
        }

        void Render::begin(const imaging::Size& size)
        {
            TLRENDER_P();

            p.size = size;

            glViewport(0, 0, p.size.w, p.size.h);
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);

            const auto viewMatrix = glm::ortho(
                0.F,
                static_cast<float>(p.size.w),
                static_cast<float>(p.size.h),
                0.F,
                -1.F,
                1.F);
            const math::Matrix4x4f mvp(
                viewMatrix[0][0], viewMatrix[0][1], viewMatrix[0][2], viewMatrix[0][3],
                viewMatrix[1][0], viewMatrix[1][1], viewMatrix[1][2], viewMatrix[1][3],
                viewMatrix[2][0], viewMatrix[2][1], viewMatrix[2][2], viewMatrix[2][3],
                viewMatrix[3][0], viewMatrix[3][1], viewMatrix[3][2], viewMatrix[3][3]);

            if (!p.meshShader)
            {
                p.meshShader = Shader::create(vertexSource(), meshFragmentSource());
            }
            p.meshShader->bind();
            p.meshShader->setUniform("transform.mvp", mvp);

            if (!p.textShader)
            {
                p.textShader = Shader::create(vertexSource(), textFragmentSource());
            }
            p.textShader->bind();
            p.textShader->setUniform("transform.mvp", mvp);

            if (!p.textureShader)
            {
                p.textureShader = Shader::create(vertexSource(), textureFragmentSource());
            }
            p.textureShader->bind();
            p.textureShader->setUniform("transform.mvp", mvp);

            if (!p.imageShader)
            {
                p.imageShader = Shader::create(vertexSource(), imageFragmentSource());
            }
            p.imageShader->bind();
            p.imageShader->setUniform("transform.mvp", mvp);

            if (!p.displayShader)
            {
                std::string source = displayFragmentSource();
                const std::string token = "// $color";
                const auto i = source.find(token);
                if (i != std::string::npos)
                {
                    source.replace(
                        i,
                        token.size(),
                        p.ocioShaderDesc ? p.ocioShaderDesc->getShaderText() : colorFunctionNoOp());
                }
                if (auto context = _context.lock())
                {
                    //context->log("tl::gl::Render", source);
                    context->log("tl::gl::Render", "Creating shader");
                }
                p.displayShader = Shader::create(vertexSource(), source);
            }
            p.displayShader->bind();
            p.displayShader->setUniform("transform.mvp", mvp);
            for (size_t i = 0; i < p.colorTextures.size(); ++i)
            {
                p.displayShader->setUniform(p.colorTextures[i].sampler, static_cast<int>(3 + i));
            }

            if (!p.dissolveShader)
            {
                p.dissolveShader = Shader::create(vertexSource(), dissolveFragmentSource());
            }

            if (!p.differenceShader)
            {
                p.differenceShader = Shader::create(vertexSource(), differenceFragmentSource());
            }
            p.differenceShader->bind();
            p.differenceShader->setUniform("transform.mvp", mvp);
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
