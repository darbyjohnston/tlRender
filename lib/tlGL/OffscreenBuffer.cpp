// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGL/OffscreenBuffer.h>

#include <tlGL/Texture.h>

#include <tlGlad/gl.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace gl
    {
        namespace
        {
            enum class Error
            {
                ColorTexture,
                RenderBuffer,
                Create,
                Init
            };

            std::string getErrorLabel(Error error)
            {
                std::string out;
                switch (error)
                {
                case Error::ColorTexture:
                    out = "Cannot create color texture";
                    break;
                case Error::RenderBuffer:
                    out = "Cannot create render buffer";
                    break;
                case Error::Create:
                    out = "Cannot create frame buffer";
                    break;
                case Error::Init:
                    out = "Cannot initialize frame buffer";
                    break;
                default: break;
                }
                return out;
            }

            GLenum getBufferInternalFormat(OffscreenDepth depth, OffscreenStencil stencil)
            {
                GLenum out = GL_NONE;
                switch (depth)
                {
                case OffscreenDepth::None:
                    switch (stencil)
                    {
                    case OffscreenStencil::_8:
                        out = GL_STENCIL_INDEX8;
                        break;
                    default:
                        break;
                    }
                    break;
                case OffscreenDepth::_24:
                    switch (stencil)
                    {
                    case OffscreenStencil::None:
                        out = GL_DEPTH_COMPONENT24;
                        break;
                    case OffscreenStencil::_8:
                        out = GL_DEPTH24_STENCIL8;
                        break;
                    default: break;
                    }
                    break;
                case OffscreenDepth::_32:
                    switch (stencil)
                    {
                    case OffscreenStencil::None:
                        out = GL_DEPTH_COMPONENT32F;
                        break;
                    case OffscreenStencil::_8:
                        out = GL_DEPTH32F_STENCIL8;
                        break;
                    default: break;
                    }
                    break;
                default: break;
                }
                return out;
            }
        }

        bool OffscreenBufferOptions::operator == (const OffscreenBufferOptions& other) const
        {
            return
                colorType == other.colorType &&
                colorFilters == other.colorFilters &&
                depth == other.depth &&
                stencil == other.stencil &&
                sampling == other.sampling;
        }

        bool OffscreenBufferOptions::operator != (const OffscreenBufferOptions& other) const
        {
            return !(*this == other);
        }

        struct OffscreenBuffer::Private
        {
            imaging::Size size;
            OffscreenBufferOptions options;
            GLuint id = 0;
            GLuint colorID = 0;
            GLuint depthStencilID = 0;
        };

        void OffscreenBuffer::_init(
            const imaging::Size& size,
            const OffscreenBufferOptions& options)
        {
            TLRENDER_P();

            p.size = size;
            p.options = options;

            GLenum target = GL_TEXTURE_2D;
            size_t samples = 0;
            switch (p.options.sampling)
            {
            case OffscreenSampling::_2:
                samples = 2;
                target = GL_TEXTURE_2D_MULTISAMPLE;
                break;
            case OffscreenSampling::_4:
                samples = 4;
                target = GL_TEXTURE_2D_MULTISAMPLE;
                break;
            case OffscreenSampling::_8:
                samples = 8;
                target = GL_TEXTURE_2D_MULTISAMPLE;
                break;
            case OffscreenSampling::_16:
                samples = 16;
                target = GL_TEXTURE_2D_MULTISAMPLE;
                break;
            default: break;
            }

            // Create the color texture.
            if (p.options.colorType != imaging::PixelType::None)
            {
                glGenTextures(1, &p.colorID);
                if (!p.colorID)
                {
                    throw std::runtime_error(getErrorLabel(Error::ColorTexture));
                }
                glBindTexture(target, p.colorID);
                switch (p.options.sampling)
                {
                case OffscreenSampling::_2:
                case OffscreenSampling::_4:
                case OffscreenSampling::_8:
                case OffscreenSampling::_16:
                    glTexImage2DMultisample(
                        target,
                        static_cast<GLsizei>(samples),
                        getTextureInternalFormat(p.options.colorType),
                        p.size.w,
                        p.size.h,
                        false);
                    break;
                default:
                    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, getTextureFilter(p.options.colorFilters.minify));
                    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, getTextureFilter(p.options.colorFilters.magnify));
                    glTexImage2D(
                        target,
                        0,
                        getTextureInternalFormat(p.options.colorType),
                        p.size.w,
                        p.size.h,
                        0,
                        getTextureFormat(p.options.colorType),
                        getTextureType(p.options.colorType),
                        0);
                    break;
                }
            }

            // Create the depth/stencil buffer.
            if (p.options.depth != OffscreenDepth::None ||
                p.options.stencil != OffscreenStencil::None)
            {
                glGenRenderbuffers(1, &p.depthStencilID);
                if (!p.depthStencilID)
                {
                    throw std::runtime_error(getErrorLabel(Error::RenderBuffer));
                }
                glBindRenderbuffer(GL_RENDERBUFFER, p.depthStencilID);
                glRenderbufferStorageMultisample(
                    GL_RENDERBUFFER,
                    static_cast<GLsizei>(samples),
                    getBufferInternalFormat(p.options.depth, p.options.stencil),
                    p.size.w,
                    p.size.h);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

            // Create the FBO.
            glGenFramebuffers(1, &p.id);
            if (!p.id)
            {
                throw std::runtime_error(getErrorLabel(Error::Create));
            }
            const OffscreenBufferBinding binding(shared_from_this());
            if (p.colorID)
            {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    target,
                    p.colorID,
                    0);
            }
            if (p.depthStencilID)
            {
                glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER,
                    p.depthStencilID);
            }
            GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (error != GL_FRAMEBUFFER_COMPLETE)
            {
                throw std::runtime_error(getErrorLabel(Error::Init));
            }
        }

        OffscreenBuffer::OffscreenBuffer() :
            _p(new Private)
        {}

        OffscreenBuffer::~OffscreenBuffer()
        {
            TLRENDER_P();
            if (p.id)
            {
                glDeleteFramebuffers(1, &p.id);
                p.id = 0;
            }
            if (p.colorID)
            {
                glDeleteTextures(1, &p.colorID);
                p.colorID = 0;
            }
            if (p.depthStencilID)
            {
                glDeleteRenderbuffers(1, &p.depthStencilID);
                p.depthStencilID = 0;
            }
        }

        std::shared_ptr<OffscreenBuffer> OffscreenBuffer::create(
            const imaging::Size& size,
            const OffscreenBufferOptions& options)
        {
            auto out = std::shared_ptr<OffscreenBuffer>(new OffscreenBuffer);
            out->_init(size, options);
            return out;
        }

        const imaging::Size& OffscreenBuffer::getSize() const
        {
            return _p->size;
        }

        imaging::SizeType OffscreenBuffer::getWidth() const
        {
            return _p->size.w;
        }

        imaging::SizeType OffscreenBuffer::getHeight() const
        {
            return _p->size.h;
        }

        const OffscreenBufferOptions& OffscreenBuffer::getOptions() const
        {
            return _p->options;
        }

        GLuint OffscreenBuffer::getID() const
        {
            return _p->id;
        }

        GLuint OffscreenBuffer::getColorID() const
        {
            return _p->colorID;
        }

        void OffscreenBuffer::bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _p->id);
        }

        bool doCreate(
            const std::shared_ptr<OffscreenBuffer>& offscreenBuffer,
            const imaging::Size& size,
            const OffscreenBufferOptions& options)
        {
            bool out = false;
            out |= size.isValid() && !offscreenBuffer;
            out |= size.isValid() && offscreenBuffer && offscreenBuffer->getSize() != size;
            out |= offscreenBuffer && offscreenBuffer->getOptions() != options;
            return out;
        }

        struct OffscreenBufferBinding::Private
        {
            std::shared_ptr<OffscreenBuffer> buffer;
            GLint previous = 0;
        };

        OffscreenBufferBinding::OffscreenBufferBinding(const std::shared_ptr<OffscreenBuffer>& buffer) :
            _p(new Private)
        {
            TLRENDER_P();
            p.buffer = buffer;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &p.previous);
            p.buffer->bind();
        }

        OffscreenBufferBinding::~OffscreenBufferBinding()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _p->previous);
        }
    }
}
