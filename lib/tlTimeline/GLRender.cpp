// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/GLRenderPrivate.h>

#include <tlGL/GL.h>
#include <tlGL/Mesh.h>
#include <tlGL/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <list>

#define _USE_MATH_DEFINES
#include <math.h>

namespace tl
{
    namespace timeline
    {
        namespace
        {
            const int pboSizeMin = 1024;
        }

        std::vector<std::shared_ptr<gl::Texture> > getTextures(
            const image::Info& info,
            const ImageFilters& imageFilters,
            size_t offset)
        {
            std::vector<std::shared_ptr<gl::Texture> > out;
            gl::TextureOptions options;
            options.filters = imageFilters;
            options.pbo = info.size.w >= pboSizeMin || info.size.h >= pboSizeMin;
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(image::Size(info.size.w / 2, info.size.h / 2), image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(image::Size(info.size.w / 2, info.size.h), image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U8);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_420P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(image::Size(info.size.w / 2, info.size.h / 2), image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_422P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(image::Size(info.size.w / 2, info.size.h), image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            case image::PixelType::YUV_444P_U16:
            {
                auto infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                infoTmp = image::Info(info.size, image::PixelType::L_U16);
                out.push_back(gl::Texture::create(infoTmp, options));
                out.push_back(gl::Texture::create(infoTmp, options));
                break;
            }
            default:
            {
                auto texture = gl::Texture::create(info, options);
                out.push_back(texture);
                break;
            }
            }
            return out;
        }

        void copyTextures(
            const std::shared_ptr<image::Image>& image,
            const std::vector<std::shared_ptr<gl::Texture> >& textures,
            size_t offset)
        {
            const auto& info = image->getInfo();
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(image->getData() + (w * h) + (w2 * h2), textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_422P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    textures[1]->copy(image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(image->getData() + (w * h) + (w2 * h), textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_444P_U8:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    textures[1]->copy(image->getData() + (w * h), textures[1]->getInfo());
                    textures[2]->copy(image->getData() + (w * h) + (w * h), textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_420P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    const std::size_t h2 = h / 2;
                    textures[1]->copy(image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(image->getData() + (w * h) * 2 + (w2 * h2) * 2, textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_422P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    const std::size_t w2 = w / 2;
                    textures[1]->copy(image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(image->getData() + (w * h) * 2 + (w2 * h) * 2, textures[2]->getInfo());
                }
                break;
            }
            case image::PixelType::YUV_444P_U16:
            {
                if (3 == textures.size())
                {
                    textures[0]->copy(image->getData(), textures[0]->getInfo());
                    const std::size_t w = info.size.w;
                    const std::size_t h = info.size.h;
                    textures[1]->copy(image->getData() + (w * h) * 2, textures[1]->getInfo());
                    textures[2]->copy(image->getData() + (w * h) * 2 + (w * h) * 2, textures[2]->getInfo());
                }
                break;
            }
            default:
                if (1 == textures.size())
                {
                    textures[0]->copy(image);
                }
                break;
            }
        }

        void setActiveTextures(
            const image::Info& info,
            const std::vector<std::shared_ptr<gl::Texture> >& textures,
            size_t offset)
        {
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_422P_U8:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_444P_U8:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_420P_U16:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_422P_U16:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            case image::PixelType::YUV_444P_U16:
                if (3 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 1 + offset));
                    textures[1]->bind();
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + 2 + offset));
                    textures[2]->bind();
                }
                break;
            default:
                if (1 == textures.size())
                {
                    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + offset));
                    textures[0]->bind();
                }
                break;
            }
        }

#if defined(TLRENDER_OCIO)
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

        OCIOData::~OCIOData()
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
#endif // TLRENDER_OCIO

        void GLRender::_init(const std::shared_ptr<system::Context>& context)
        {
            IRender::_init(context);
            TLRENDER_P();

            p.glyphTextureAtlas = gl::TextureAtlas::create(
                1,
                4096,
                image::PixelType::L_U8,
                ImageFilter::Linear);

            p.logTimer = std::chrono::steady_clock::now();
        }

        GLRender::GLRender() :
            _p(new Private)
        {}

        GLRender::~GLRender()
        {}

        std::shared_ptr<GLRender> GLRender::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<GLRender>(new GLRender);
            out->_init(context);
            return out;
        }

        void GLRender::begin(
            const math::Size2i& renderSize,
            const OCIOOptions& ocioOptions,
            const LUTOptions& lutOptions,
            const RenderOptions& renderOptions)
        {
            TLRENDER_P();

            p.timer = std::chrono::steady_clock::now();

            p.renderSize = renderSize;
            _setOCIO(ocioOptions);
            _setLUT(lutOptions);
            p.renderOptions = renderOptions;
            p.textureCache.setMax(renderOptions.textureCacheByteCount);

            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);

            if (!p.shaders["rect"])
            {
                p.shaders["rect"] = gl::Shader::create(
                    vertexSource(),
                    meshFragmentSource());
            }
            if (!p.shaders["mesh"])
            {
                p.shaders["mesh"] = gl::Shader::create(
                    vertexSource(),
                    meshFragmentSource());
            }
            if (!p.shaders["colorMesh"])
            {
                p.shaders["colorMesh"] = gl::Shader::create(
                    colorMeshVertexSource(),
                    colorMeshFragmentSource());
            }
            if (!p.shaders["text"])
            {
                p.shaders["text"] = gl::Shader::create(
                    vertexSource(),
                    textFragmentSource());
            }
            if (!p.shaders["texture"])
            {
                p.shaders["texture"] = gl::Shader::create(
                    vertexSource(),
                    textureFragmentSource());
            }
            if (!p.shaders["image"])
            {
                p.shaders["image"] = gl::Shader::create(
                    vertexSource(),
                    imageFragmentSource());
            }
            if (!p.shaders["wipe"])
            {
                p.shaders["wipe"] = gl::Shader::create(
                    vertexSource(),
                    meshFragmentSource());
            }
            if (!p.shaders["overlay"])
            {
                p.shaders["overlay"] = gl::Shader::create(
                    vertexSource(),
                    textureFragmentSource());
            }
            if (!p.shaders["difference"])
            {
                p.shaders["difference"] = gl::Shader::create(
                    vertexSource(),
                    differenceFragmentSource());
            }
            if (!p.shaders["dissolve"])
            {
                p.shaders["dissolve"] = gl::Shader::create(
                    vertexSource(),
                    textureFragmentSource());
            }
            if (!p.shaders["display"])
            {
                std::string ocioDef;
                std::string ocio;
                std::string lutDef;
                std::string lut;

#if defined(TLRENDER_OCIO)
                if (p.ocioData && p.ocioData->shaderDesc)
                {
                    ocioDef = p.ocioData->shaderDesc->getShaderText();
                    ocio = "outColor = ocioFunc(outColor);";
                }
                if (p.lutData && p.lutData->shaderDesc)
                {
                    lutDef = p.lutData->shaderDesc->getShaderText();
                    lut = "outColor = lutFunc(outColor);";
                }
#endif // TLRENDER_OCIO
                const std::string source = displayFragmentSource(
                    ocioDef,
                    ocio,
                    lutDef,
                    lut,
                    p.lutOptions.order);
                if (auto context = _context.lock())
                {
                    //context->log("tl::gl::GLRender", source);
                    context->log("tl::gl::GLRender", "Creating display shader");
                }
                p.shaders["display"] = gl::Shader::create(vertexSource(), source);
            }
            p.shaders["display"]->bind();
            size_t texturesOffset = 1;
#if defined(TLRENDER_OCIO)
            if (p.ocioData)
            {
                for (size_t i = 0; i < p.ocioData->textures.size(); ++i)
                {
                    p.shaders["display"]->setUniform(
                        p.ocioData->textures[i].sampler,
                        static_cast<int>(texturesOffset + i));
                }
                texturesOffset += p.ocioData->textures.size();
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
#endif // TLRENDER_OCIO

            p.vbos["rect"] = gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32);
            p.vaos["rect"] = gl::VAO::create(p.vbos["rect"]->getType(), p.vbos["rect"]->getID());
            p.vbos["texture"] = gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32_UV_U16);
            p.vaos["texture"] = gl::VAO::create(p.vbos["texture"]->getType(), p.vbos["texture"]->getID());
            p.vbos["image"] = gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32_UV_U16);
            p.vaos["image"] = gl::VAO::create(p.vbos["image"]->getType(), p.vbos["image"]->getID());
            p.vbos["wipe"] = gl::VBO::create(1 * 3, gl::VBOType::Pos2_F32);
            p.vaos["wipe"] = gl::VAO::create(p.vbos["wipe"]->getType(), p.vbos["wipe"]->getID());
            p.vbos["video"] = gl::VBO::create(2 * 3, gl::VBOType::Pos2_F32_UV_U16);
            p.vaos["video"] = gl::VAO::create(p.vbos["video"]->getType(), p.vbos["video"]->getID());

            setViewport(math::Box2i(0, 0, renderSize.w, renderSize.h));
            if (renderOptions.clear)
            {
                clearViewport(renderOptions.clearColor);
            }
            setTransform(math::ortho(
                0.F,
                static_cast<float>(renderSize.w),
                static_cast<float>(renderSize.h),
                0.F,
                -1.F,
                1.F));
        }

        void GLRender::end()
        {
            TLRENDER_P();

            //! \bug Should these be reset periodically?
            //p.glyphIDs.clear();
            //p.vbos["mesh"].reset();
            //p.vaos["mesh"].reset();
            //p.vbos["text"].reset();
            //p.vaos["text"].reset();

            const auto now = std::chrono::steady_clock::now();
            const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - p.timer);
            p.currentStats.time = diff.count();
            p.stats.push_back(p.currentStats);
            p.currentStats = Private::Stats();
            while (p.stats.size() > 60)
            {
                p.stats.pop_front();
            }

            const std::chrono::duration<float> logDiff = now - p.logTimer;
            if (logDiff.count() > 10.F)
            {
                p.logTimer = now;
                if (auto context = _context.lock())
                {
                    Private::Stats average;
                    const size_t size = p.stats.size();
                    if (size > 0)
                    {
                        for (const auto& i : p.stats)
                        {
                            average.time += i.time;
                            average.rects += i.rects;
                            average.meshes += i.meshes;
                            average.meshTriangles += i.meshTriangles;
                            average.text += i.text;
                            average.textTriangles += i.textTriangles;
                            average.textures += i.textures;
                            average.images += i.images;
                        }
                        average.time /= p.stats.size();
                        average.rects /= p.stats.size();
                        average.meshes /= p.stats.size();
                        average.meshTriangles /= p.stats.size();
                        average.text /= p.stats.size();
                        average.textTriangles /= p.stats.size();
                        average.textures /= p.stats.size();
                        average.images /= p.stats.size();
                    }

                    context->log(
                        string::Format("tl::timeline::GLRender {0}").arg(this),
                        string::Format(
                            "\n"
                            "    Average render time: {0}ms\n"
                            "    Average rectangle count: {1}\n"
                            "    Average mesh count: {2}\n"
                            "    Average mesh triangles: {3}\n"
                            "    Average text count: {4}\n"
                            "    Average text triangles: {5}\n"
                            "    Average texture count: {6}\n"
                            "    Average image count: {7}\n"
                            "    Glyph texture atlas: {8}%\n"
                            "    Glyph IDs: {9}").
                        arg(average.time).
                        arg(average.rects).
                        arg(average.meshes).
                        arg(average.meshTriangles).
                        arg(average.text).
                        arg(average.textTriangles).
                        arg(average.textures).
                        arg(average.images).
                        arg(p.glyphTextureAtlas->getPercentageUsed()).
                        arg(p.glyphIDs.size()));
                }
            }
        }

        math::Size2i GLRender::getRenderSize() const
        {
            return _p->renderSize;
        }

        void GLRender::setRenderSize(const math::Size2i& value)
        {
            _p->renderSize = value;
        }

        math::Box2i GLRender::getViewport() const
        {
            return _p->viewport;
        }

        void GLRender::setViewport(const math::Box2i& value)
        {
            TLRENDER_P();
            p.viewport = value;
            glViewport(
                value.x(),
                p.renderSize.h - value.h() - value.y(),
                value.w(),
                value.h());
        }

        void GLRender::clearViewport(const image::Color4f& value)
        {
            glClearColor(value.r, value.g, value.b, value.a);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        bool GLRender::getClipRectEnabled() const
        {
            return _p->clipRectEnabled;
        }

        void GLRender::setClipRectEnabled(bool value)
        {
            TLRENDER_P();
            p.clipRectEnabled = value;
            if (p.clipRectEnabled)
            {
                glEnable(GL_SCISSOR_TEST);
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }
        }

        math::Box2i GLRender::getClipRect() const
        {
            return _p->clipRect;
        }

        void GLRender::setClipRect(const math::Box2i& value)
        {
            TLRENDER_P();
            p.clipRect = value;
            if (value.w() > 0 && value.h() > 0)
            {
                glScissor(
                    value.x(),
                    p.renderSize.h - value.h() - value.y(),
                    value.w(),
                    value.h());
            }
        }

        math::Matrix4x4f GLRender::getTransform() const
        {
            return _p->transform;
        }

        void GLRender::setTransform(const math::Matrix4x4f& value)
        {
            TLRENDER_P();
            p.transform = value;
            for (auto i : p.shaders)
            {
                i.second->bind();
                i.second->setUniform("transform.mvp", value);
            }
        }

        namespace
        {
#if defined(TLRENDER_OCIO)
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
#endif // TLRENDER_OCIO
        }

        void GLRender::_setOCIO(const OCIOOptions& value)
        {
            TLRENDER_P();
            if (value == p.ocioOptions)
                return;

#if defined(TLRENDER_OCIO)
            p.ocioData.reset();
#endif // TLRENDER_OCIO

            p.ocioOptions = value;

#if defined(TLRENDER_OCIO)
            if (p.ocioOptions.enabled &&
                !p.ocioOptions.input.empty() &&
                !p.ocioOptions.display.empty() &&
                !p.ocioOptions.view.empty())
            {
                p.ocioData.reset(new OCIOData);

                if (!p.ocioOptions.fileName.empty())
                {
                    p.ocioData->config = OCIO::Config::CreateFromFile(p.ocioOptions.fileName.c_str());
                }
                else
                {
                    p.ocioData->config = OCIO::GetCurrentConfig();
                }
                if (!p.ocioData->config)
                {
                    throw std::runtime_error("Cannot get OCIO configuration");
                }

                p.ocioData->transform = OCIO::DisplayViewTransform::Create();
                if (!p.ocioData->transform)
                {
                    p.ocioData.reset();
                    throw std::runtime_error("Cannot create OCIO transform");
                }
                p.ocioData->transform->setSrc(p.ocioOptions.input.c_str());
                p.ocioData->transform->setDisplay(p.ocioOptions.display.c_str());
                p.ocioData->transform->setView(p.ocioOptions.view.c_str());

                p.ocioData->lvp = OCIO::LegacyViewingPipeline::Create();
                if (!p.ocioData->lvp)
                {
                    p.ocioData.reset();
                    throw std::runtime_error("Cannot create OCIO viewing pipeline");
                }
                p.ocioData->lvp->setDisplayViewTransform(p.ocioData->transform);
                p.ocioData->lvp->setLooksOverrideEnabled(true);
                p.ocioData->lvp->setLooksOverride(p.ocioOptions.look.c_str());

                p.ocioData->processor = p.ocioData->lvp->getProcessor(
                    p.ocioData->config,
                    p.ocioData->config->getCurrentContext());
                if (!p.ocioData->processor)
                {
                    p.ocioData.reset();
                    throw std::runtime_error("Cannot get OCIO processor");
                }
                p.ocioData->gpuProcessor = p.ocioData->processor->getDefaultGPUProcessor();
                if (!p.ocioData->gpuProcessor)
                {
                    p.ocioData.reset();
                    throw std::runtime_error("Cannot get OCIO GPU processor");
                }
                p.ocioData->shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
                if (!p.ocioData->shaderDesc)
                {
                    p.ocioData.reset();
                    throw std::runtime_error("Cannot create OCIO shader description");
                }
                p.ocioData->shaderDesc->setLanguage(OCIO::GPU_LANGUAGE_GLSL_4_0);
                p.ocioData->shaderDesc->setFunctionName("ocioFunc");
                p.ocioData->shaderDesc->setResourcePrefix("ocio");
                p.ocioData->gpuProcessor->extractGpuShaderInfo(p.ocioData->shaderDesc);

                // Create 3D textures.
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                const unsigned num3DTextures = p.ocioData->shaderDesc->getNum3DTextures();
                unsigned currentTexture = 0;
                for (unsigned i = 0; i < num3DTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned edgelen = 0;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.ocioData->shaderDesc->get3DTexture(i, textureName, samplerName, edgelen, interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        0 == edgelen)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.ocioData->shaderDesc->get3DTextureValues(i, values);
                    if (!values)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error("The OCIO texture values are missing");
                    }

                    unsigned textureId = 0;
                    glGenTextures(1, &textureId);
                    glBindTexture(GL_TEXTURE_3D, textureId);
                    setTextureParameters(GL_TEXTURE_3D, interpolation);
                    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, edgelen, edgelen, edgelen, 0, GL_RGB, GL_FLOAT, values);
                    p.ocioData->textures.push_back(OCIOTexture(textureId, textureName, samplerName, GL_TEXTURE_3D));
                }

                // Create 1D textures.
                const unsigned numTextures = p.ocioData->shaderDesc->getNumTextures();
                for (unsigned i = 0; i < numTextures; ++i, ++currentTexture)
                {
                    const char* textureName = nullptr;
                    const char* samplerName = nullptr;
                    unsigned width = 0;
                    unsigned height = 0;
                    OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
                    OCIO::GpuShaderCreator::TextureDimensions dimensions = OCIO::GpuShaderDesc::TEXTURE_1D;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.ocioData->shaderDesc->getTexture(
                        i,
                        textureName,
                        samplerName,
                        width,
                        height,
                        channel,
                        dimensions,
                        interpolation);
                    if (!textureName ||
                        !*textureName ||
                        !samplerName ||
                        !*samplerName ||
                        width == 0)
                    {
                        p.ocioData.reset();
                        throw std::runtime_error("The OCIO texture data is corrupted");
                    }

                    const float* values = nullptr;
                    p.ocioData->shaderDesc->getTextureValues(i, values);
                    if (!values)
                    {
                        p.ocioData.reset();
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
                    switch (dimensions)
                    {
                    case OCIO::GpuShaderDesc::TEXTURE_1D:
                        glBindTexture(GL_TEXTURE_1D, textureId);
                        setTextureParameters(GL_TEXTURE_1D, interpolation);
                        glTexImage1D(GL_TEXTURE_1D, 0, internalformat, width, 0, format, GL_FLOAT, values);
                        break;
                    case OCIO::GpuShaderDesc::TEXTURE_2D:
                        glBindTexture(GL_TEXTURE_2D, textureId);
                        setTextureParameters(GL_TEXTURE_2D, interpolation);
                        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_FLOAT, values);
                        break;
                    }
                    p.ocioData->textures.push_back(OCIOTexture(
                        textureId,
                        textureName,
                        samplerName,
                        (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }
#endif // TLRENDER_OCIO

            p.shaders["display"].reset();
        }

        void GLRender::_setLUT(const LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;

#if defined(TLRENDER_OCIO)
            p.lutData.reset();
#endif // TLRENDER_OCIO

            p.lutOptions = value;

#if defined(TLRENDER_OCIO)
            if (p.lutOptions.enabled && !p.lutOptions.fileName.empty())
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
                    OCIO::GpuShaderDesc::TextureDimensions dimensions = OCIO::GpuShaderDesc::TEXTURE_1D;
                    OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
                    p.lutData->shaderDesc->getTexture(
                        i, textureName,
                        samplerName,
                        width,
                        height,
                        channel,
                        dimensions,
                        interpolation);
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
                    switch (dimensions)
                    {
                    case OCIO::GpuShaderDesc::TEXTURE_1D:
                        glBindTexture(GL_TEXTURE_1D, textureId);
                        setTextureParameters(GL_TEXTURE_1D, interpolation);
                        glTexImage1D(GL_TEXTURE_1D, 0, internalformat, width, 0, format, GL_FLOAT, values);
                        break;
                    case OCIO::GpuShaderDesc::TEXTURE_2D:
                        glBindTexture(GL_TEXTURE_2D, textureId);
                        setTextureParameters(GL_TEXTURE_2D, interpolation);
                        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_FLOAT, values);
                        break;
                    }
                    p.lutData->textures.push_back(OCIOTexture(
                        textureId,
                        textureName,
                        samplerName,
                        (height > 1) ? GL_TEXTURE_2D : GL_TEXTURE_1D));
                }
            }
#endif // TLRENDER_OCIO

            p.shaders["display"].reset();
        }
    }
}
