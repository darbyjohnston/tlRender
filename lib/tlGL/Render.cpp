// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlGL/Mesh.h>
#include <tlGL/Util.h>

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

            std::vector<std::shared_ptr<Texture> > getTextures(
                const imaging::Info& info,
                const timeline::ImageFilters& imageFilters,
                size_t offset)
            {
                std::vector<std::shared_ptr<Texture> > out;
                TextureOptions options;
                options.minifyFilter = getTextureFilter(imageFilters.minify);
                options.magnifyFilter = getTextureFilter(imageFilters.magnify);
                options.pbo = true;
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

        std::vector<std::shared_ptr<Texture> > TextureCache::get(
            const imaging::Info& info,
            const timeline::ImageFilters& imageFilters,
            size_t offset)
        {
            std::vector<std::shared_ptr<Texture> > out;
            const auto i = std::find_if(
                _cache.begin(), 
                _cache.end(),
                [info, imageFilters](const TextureData& value)
                {
                    return info == value.info &&
                        imageFilters == value.imageFilters;
                });
            if (i != _cache.end())
            {
                out = i->texture;
                _cache.erase(i);
            }
            else
            {
                out = getTextures(info, imageFilters, offset);
            }
            return out;
        }

        void TextureCache::add(
            const imaging::Info& info,
            const timeline::ImageFilters& imageFilters,
            const std::vector<std::shared_ptr<Texture> >& textures)
        {
            _cache.push_front({ info, imageFilters, textures });
            _cacheUpdate();
        }

        void TextureCache::_cacheUpdate()
        {
            while (_cache.size() > _size)
            {
                _cache.pop_back();
            }
        }

        OCIOTexture::OCIOTexture(
            unsigned id,
            std::string name,
            std::string sampler,
            unsigned type) :
            id(id),
            name(name),
            sampler(sampler),
            type(type)
        {}

        OCIOColorConfigData::~OCIOColorConfigData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                glDeleteTextures(1, &textures[i].id);
            }
        }

        OCIOLUTData::~OCIOLUTData()
        {
            for (size_t i = 0; i < textures.size(); ++i)
            {
                glDeleteTextures(1, &textures[i].id);
            }
        }

        void Render::_init(const std::shared_ptr<system::Context>& context)
        {
            IRender::_init(context);
        }

        Render::Render() :
            _p(new Private)
        {}

        Render::~Render()
        {}

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

        void Render::setColorConfig(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            if (value == p.colorConfigOptions)
                return;

            p.colorConfigData.reset();

            p.colorConfigOptions = value;

            if (!p.colorConfigOptions.input.empty() &&
                !p.colorConfigOptions.display.empty() &&
                !p.colorConfigOptions.view.empty())
            {
                p.colorConfigData.reset(new OCIOColorConfigData);

                if (!p.colorConfigOptions.fileName.empty())
                {
                    p.colorConfigData->config = OCIO::Config::CreateFromFile(p.colorConfigOptions.fileName.c_str());
                }
                else
                {
                    p.colorConfigData->config = OCIO::GetCurrentConfig();
                }
                if (!p.colorConfigData->config)
                {
                    throw std::runtime_error("Cannot get OCIO configuration");
                }

                p.colorConfigData->transform = OCIO::DisplayViewTransform::Create();
                if (!p.colorConfigData->transform)
                {
                    p.colorConfigData.reset();
                    throw std::runtime_error("Cannot create OCIO transform");
                }
                p.colorConfigData->transform->setSrc(p.colorConfigOptions.input.c_str());
                p.colorConfigData->transform->setDisplay(p.colorConfigOptions.display.c_str());
                p.colorConfigData->transform->setView(p.colorConfigOptions.view.c_str());

                p.colorConfigData->lvp = OCIO::LegacyViewingPipeline::Create();
                if (!p.colorConfigData->lvp)
                {
                    p.colorConfigData.reset();
                    throw std::runtime_error("Cannot create OCIO viewing pipeline");
                }
                p.colorConfigData->lvp->setDisplayViewTransform(p.colorConfigData->transform);
                p.colorConfigData->lvp->setLooksOverrideEnabled(true);
                p.colorConfigData->lvp->setLooksOverride(p.colorConfigOptions.look.c_str());

                p.colorConfigData->processor = p.colorConfigData->lvp->getProcessor(
                    p.colorConfigData->config,
                    p.colorConfigData->config->getCurrentContext());
                if (!p.colorConfigData->processor)
                {
                    p.colorConfigData.reset();
                    throw std::runtime_error("Cannot get OCIO processor");
                }
                p.colorConfigData->gpuProcessor = p.colorConfigData->processor->getDefaultGPUProcessor();
                if (!p.colorConfigData->gpuProcessor)
                {
                    p.colorConfigData.reset();
                    throw std::runtime_error("Cannot get OCIO GPU processor");
                }
                p.colorConfigData->shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                if (!p.colorConfigData->shaderDesc)
                {
                    p.colorConfigData.reset();
                    throw std::runtime_error("Cannot create OCIO shader description");
                }
                p.colorConfigData->shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
                p.colorConfigData->shaderDesc->setFunctionName("colorConfigFunc");
                p.colorConfigData->shaderDesc->setResourcePrefix("colorConfig");
                p.colorConfigData->gpuProcessor->extractGpuShaderInfo(p.colorConfigData->shaderDesc);

                // Create 3D textures.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                const unsigned num3DTextures = p.colorConfigData->shaderDesc->getNum3DTextures();
                unsigned currentTexture = 0;
                for (unsigned i = 0; i < num3DTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned edgelen = 0;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.colorConfigData->shaderDesc->get3DTexture(i, textureName, samplerName, edgelen, interpolation);
                    if (!textureName  ||
                        !*textureName ||
                        !samplerName  ||
                        !*samplerName ||
                        0 == edgelen)
                    {
                        p.colorConfigData.reset();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.colorConfigData->shaderDesc->get3DTextureValues(i, values);
                    if (!values)
                    {
                        p.colorConfigData.reset();
                        throw std::runtime_error("The OCIO texture values are missing");
                    }

                    unsigned textureId = 0;
                    glGenTextures(1, &textureId);
                    glBindTexture(GL_TEXTURE_3D, textureId);
                    setTextureParameters(GL_TEXTURE_3D, interpolation);
                    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, edgelen, edgelen, edgelen, 0, GL_RGB, GL_FLOAT, values);
                    p.colorConfigData->textures.push_back(OCIOTexture(textureId, textureName, samplerName, GL_TEXTURE_3D));
                }

                // Create 1D textures.
                const unsigned numTextures = p.colorConfigData->shaderDesc->getNumTextures();
                for (unsigned i = 0; i < numTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned width = 0;
                    unsigned height = 0;
                    OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.colorConfigData->shaderDesc->getTexture(i, textureName, samplerName, width, height, channel, interpolation);
                    if (!textureName  ||
                        !*textureName ||
                        !samplerName  ||
                        !*samplerName ||
                        width == 0)
                    {
                        p.colorConfigData.reset();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.colorConfigData->shaderDesc->getTextureValues(i, values);
                    if (!values)
                    {
                        p.colorConfigData.reset();
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
                    p.colorConfigData->textures.push_back(OCIOTexture(
                        textureId,
                        textureName,
                        samplerName,
                        (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }

            p.shaders["display"].reset();
        }

        void Render::setLUT(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;

            p.lutData.reset();

            p.lutOptions = value;

            if (!p.lutOptions.fileName.empty())
            {
                p.lutData.reset(new OCIOLUTData);

                p.lutData->config = OCIO::Config::CreateRaw();
                if (!p.lutData->config)
                {
                    throw std::runtime_error("Cannot create OCIO configuration");
                }

                p.lutData->transform = OCIO::FileTransform::Create();
                if (!p.lutData->transform)
                {
                    p.lutData.reset();
                    throw std::runtime_error("Cannot create OCIO transform");
                }
                p.lutData->transform->setSrc(p.lutOptions.fileName.c_str());
                p.lutData->transform->validate();

                p.lutData->processor = p.lutData->config->getProcessor(p.lutData->transform);
                if (!p.lutData->processor)
                {
                    p.lutData.reset();
                    throw std::runtime_error("Cannot get OCIO processor");
                }
                p.lutData->gpuProcessor = p.lutData->processor->getDefaultGPUProcessor();
                if (!p.lutData->gpuProcessor)
                {
                    p.lutData.reset();
                    throw std::runtime_error("Cannot get OCIO GPU processor");
                }
                p.lutData->shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                if (!p.lutData->shaderDesc)
                {
                    p.lutData.reset();
                    throw std::runtime_error("Cannot create OCIO shader description");
                }
                p.lutData->shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
                p.lutData->shaderDesc->setFunctionName("lutFunc");
                p.lutData->shaderDesc->setResourcePrefix("lut");
                p.lutData->gpuProcessor->extractGpuShaderInfo(p.lutData->shaderDesc);

                // Create 3D textures.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                const unsigned num3DTextures = p.lutData->shaderDesc->getNum3DTextures();
                unsigned currentTexture = 0;
                for (unsigned i = 0; i < num3DTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned edgelen = 0;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.lutData->shaderDesc->get3DTexture(i, textureName, samplerName, edgelen, interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        0 == edgelen)
                    {
                        p.lutData.reset();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.lutData->shaderDesc->get3DTextureValues(i, values);
                    if (!values)
                    {
                        p.lutData.reset();
                        throw std::runtime_error("The OCIO texture values are missing");
                    }

                    unsigned textureId = 0;
                    glGenTextures(1, &textureId);
                    glBindTexture(GL_TEXTURE_3D, textureId);
                    setTextureParameters(GL_TEXTURE_3D, interpolation);
                    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, edgelen, edgelen, edgelen, 0, GL_RGB, GL_FLOAT, values);
                    p.lutData->textures.push_back(OCIOTexture(textureId, textureName, samplerName, GL_TEXTURE_3D));
                }

                // Create 1D textures.
                const unsigned numTextures = p.lutData->shaderDesc->getNumTextures();
                for (unsigned i = 0; i < numTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned width = 0;
                    unsigned height = 0;
                    OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.lutData->shaderDesc->getTexture(i, textureName, samplerName, width, height, channel, interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        width == 0)
                    {
                        p.lutData.reset();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.lutData->shaderDesc->getTextureValues(i, values);
                    if (!values)
                    {
                        p.lutData.reset();
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
                    p.lutData->textures.push_back(OCIOTexture(
                        textureId,
                        textureName,
                        samplerName,
                        (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }

            p.shaders["display"].reset();
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

            if (!p.shaders["mesh"])
            {
                p.shaders["mesh"] = Shader::create(vertexSource(), meshFragmentSource());
            }
            p.shaders["mesh"]->bind();
            p.shaders["mesh"]->setUniform("transform.mvp", mvp);

            if (!p.shaders["text"])
            {
                p.shaders["text"] = Shader::create(vertexSource(), textFragmentSource());
            }
            p.shaders["text"]->bind();
            p.shaders["text"]->setUniform("transform.mvp", mvp);

            if (!p.shaders["texture"])
            {
                p.shaders["texture"] = Shader::create(vertexSource(), textureFragmentSource());
            }
            p.shaders["texture"]->bind();
            p.shaders["texture"]->setUniform("transform.mvp", mvp);

            if (!p.shaders["image"])
            {
                p.shaders["image"] = Shader::create(vertexSource(), imageFragmentSource());
            }
            p.shaders["image"]->bind();
            p.shaders["image"]->setUniform("transform.mvp", mvp);

            if (!p.shaders["display"])
            {
                std::string colorConfigDef;
                std::string colorConfig;
                std::string lutDef;
                std::string lut;
                if (p.colorConfigData && p.colorConfigData->shaderDesc)
                {
                    colorConfigDef = p.colorConfigData->shaderDesc->getShaderText();
                    colorConfig = "fColor = colorConfigFunc(fColor);";
                }
                if (p.lutData && p.lutData->shaderDesc)
                {
                    lutDef = p.lutData->shaderDesc->getShaderText();
                    lut = "fColor = lutFunc(fColor);";
                }
                std::string source = displayFragmentSource(
                    colorConfigDef,
                    colorConfig,
                    lutDef,
                    lut,
                    p.lutOptions.order);
                if (auto context = _context.lock())
                {
                    //context->log("tl::gl::Render", source);
                    context->log("tl::gl::Render", "Creating display shader");
                }
                p.shaders["display"] = Shader::create(vertexSource(), source);
            }
            p.shaders["display"]->bind();
            p.shaders["display"]->setUniform("transform.mvp", mvp);
            size_t texturesOffset = 1;
            if (p.colorConfigData)
            {
                for (size_t i = 0; i < p.colorConfigData->textures.size(); ++i)
                {
                    p.shaders["display"]->setUniform(
                        p.colorConfigData->textures[i].sampler,
                        static_cast<int>(texturesOffset + i));
                }
                texturesOffset += p.colorConfigData->textures.size();
            }
            if (p.lutData)
            {
                for (size_t i = 0; i < p.lutData->textures.size(); ++i)
                {
                    p.shaders["display"]->setUniform(
                        p.lutData->textures[i].sampler,
                        static_cast<int>(texturesOffset + i));
                }
                texturesOffset += p.lutData->textures.size();
            }

            if (!p.shaders["dissolve"])
            {
                p.shaders["dissolve"] = Shader::create(vertexSource(), dissolveFragmentSource());
            }

            if (!p.shaders["difference"])
            {
                p.shaders["difference"] = Shader::create(vertexSource(), differenceFragmentSource());
            }
            p.shaders["difference"]->bind();
            p.shaders["difference"]->setUniform("transform.mvp", mvp);

            p.vbos["rect"] = VBO::create(2 * 3, VBOType::Pos2_F32);
            p.vaos["rect"] = VAO::create(p.vbos["rect"]->getType(), p.vbos["rect"]->getID());
            p.vbos["text"] = VBO::create(2 * 3, VBOType::Pos2_F32_UV_U16);
            p.vaos["text"] = VAO::create(p.vbos["text"]->getType(), p.vbos["text"]->getID());
            p.vbos["image"] = VBO::create(2 * 3, VBOType::Pos2_F32_UV_U16);
            p.vaos["image"] = VAO::create(p.vbos["image"]->getType(), p.vbos["image"]->getID());
            p.vbos["wipe"] = VBO::create(2 * 3, VBOType::Pos2_F32);
            p.vaos["wipe"] = VAO::create(p.vbos["wipe"]->getType(), p.vbos["wipe"]->getID());
            p.vbos["video"] = VBO::create(2 * 3, VBOType::Pos2_F32_UV_U16);
            p.vaos["video"] = VAO::create(p.vbos["video"]->getType(), p.vbos["video"]->getID());
        }

        void Render::end()
        {}
    }
}
