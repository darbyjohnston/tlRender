// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlRenderGL/OffscreenBuffer.h>

#include <tlRenderGL/Texture.h>

#include <array>
#include <sstream>

using namespace tl::core;

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

        void OffscreenBuffer::_init(
            const imaging::Size& size,
            const OffscreenBufferOptions& options)
        {
            _size = size;
            _options = options;

            GLenum target = GL_TEXTURE_2D;
            size_t samples = 0;
            switch (_options.sampling)
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
            if (_options.colorType != imaging::PixelType::None)
            {
                glGenTextures(1, &_colorID);
                if (!_colorID)
                {
                    throw std::runtime_error(getErrorLabel(Error::ColorTexture));
                }
                glBindTexture(target, _colorID);
                switch (_options.sampling)
                {
                case OffscreenSampling::_2:
                case OffscreenSampling::_4:
                case OffscreenSampling::_8:
                case OffscreenSampling::_16:
                    glTexImage2DMultisample(
                        target,
                        static_cast<GLsizei>(samples),
                        getTextureInternalFormat(_options.colorType),
                        _size.w,
                        _size.h,
                        false);
                    break;
                default:
                    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, _options.colorMag);
                    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, _options.colorMin);
                    glTexImage2D(
                        target,
                        0,
                        getTextureInternalFormat(_options.colorType),
                        _size.w,
                        _size.h,
                        0,
                        getTextureFormat(_options.colorType),
                        getTextureType(_options.colorType),
                        0);
                    break;
                }
            }

            // Create the depth/stencil buffer.
            if (_options.depth != OffscreenDepth::None ||
                _options.stencil != OffscreenStencil::None)
            {
                glGenRenderbuffers(1, &_depthStencilID);
                if (!_depthStencilID)
                {
                    throw std::runtime_error(getErrorLabel(Error::RenderBuffer));
                }
                glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilID);
                glRenderbufferStorageMultisample(
                    GL_RENDERBUFFER,
                    static_cast<GLsizei>(samples),
                    getBufferInternalFormat(_options.depth, _options.stencil),
                    _size.w,
                    _size.h);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

            // Create the FBO.
            glGenFramebuffers(1, &_id);
            if (!_id)
            {
                throw std::runtime_error(getErrorLabel(Error::Create));
            }
            const OffscreenBufferBinding binding(shared_from_this());
            if (_colorID)
            {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    target,
                    _colorID,
                    0);
            }
            if (_depthStencilID)
            {
                glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_STENCIL_ATTACHMENT,
                    GL_RENDERBUFFER,
                    _depthStencilID);
            }
            GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (error != GL_FRAMEBUFFER_COMPLETE)
            {
                throw std::runtime_error(getErrorLabel(Error::Init));
            }
        }

        OffscreenBuffer::OffscreenBuffer()
        {}

        OffscreenBuffer::~OffscreenBuffer()
        {
            if (_id)
            {
                glDeleteFramebuffers(1, &_id);
                _id = 0;
            }
            if (_colorID)
            {
                glDeleteTextures(1, &_colorID);
                _colorID = 0;
            }
            if (_depthStencilID)
            {
                glDeleteRenderbuffers(1, &_depthStencilID);
                _depthStencilID = 0;
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
            return _size;
        }

        const OffscreenBufferOptions& OffscreenBuffer::getOptions() const
        {
            return _options;
        }

        GLuint OffscreenBuffer::getID() const
        {
            return _id;
        }

        GLuint OffscreenBuffer::getColorID() const
        {
            return _colorID;
        }

        void OffscreenBuffer::bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _id);
        }

        OffscreenBufferBinding::OffscreenBufferBinding(const std::shared_ptr<OffscreenBuffer>& buffer) :
            _buffer(buffer)
        {
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_previous);
            _buffer->bind();
        }

        OffscreenBufferBinding::~OffscreenBufferBinding()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _previous);
        }
    }
}
