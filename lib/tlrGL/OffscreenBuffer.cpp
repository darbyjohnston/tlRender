// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "OffscreenBuffer.h"

#include "Texture.h"

#include <array>
#include <sstream>

namespace tlr
{
    namespace gl
    {
        namespace
        {
            enum class Error
            {
                ColorTexture,
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
        }

        void OffscreenBuffer::_init(
            const imaging::Size& size,
            imaging::PixelType colorType,
            OffscreenSampling sampling)
        {
            _size = size;
            _colorType = colorType;
            _sampling = sampling;

            GLenum target = GL_TEXTURE_2D;
            size_t samples = 0;
            switch (sampling)
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

            if (colorType != imaging::PixelType::None)
            {
                // Create the color texture.
                //! \bug Fall back to a regular offscreen buffer if multi-sampling is not available.
                glGenTextures(1, &_colorID);
                if (!_colorID)
                {
                    throw std::runtime_error(getErrorLabel(Error::ColorTexture));
                }
                glBindTexture(target, _colorID);
                switch (sampling)
                {
                case OffscreenSampling::_2:
                case OffscreenSampling::_4:
                case OffscreenSampling::_8:
                case OffscreenSampling::_16:
                    glTexImage2DMultisample(
                        target,
                        static_cast<GLsizei>(samples),
                        getTextureInternalFormat(colorType),
                        size.w,
                        size.h,
                        false);
                    break;
                default:
                    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(
                        target,
                        0,
                        getTextureInternalFormat(colorType),
                        size.w,
                        size.h,
                        0,
                        getTextureFormat(colorType),
                        getTextureType(colorType),
                        0);
                    break;
                }
            }

            // Create the FBO.
            glGenFramebuffers(1, &_id);
            if (!_id)
            {
                throw std::runtime_error(getErrorLabel(Error::Create));
            }
            const OffscreenBufferBinding binding(shared_from_this());
            if (colorType != imaging::PixelType::None)
            {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    target,
                    _colorID,
                    0);
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
        }

        std::shared_ptr<OffscreenBuffer> OffscreenBuffer::create(
            const imaging::Size& size,
            imaging::PixelType colorType)
        {
            auto out = std::shared_ptr<OffscreenBuffer>(new OffscreenBuffer);
            out->_init(
                size,
                colorType,
                OffscreenSampling::None);
            return out;
        }

        std::shared_ptr<OffscreenBuffer> OffscreenBuffer::create(
            const imaging::Size& size,
            imaging::PixelType colorType,
            OffscreenSampling sampling)
        {
            auto out = std::shared_ptr<OffscreenBuffer>(new OffscreenBuffer);
            out->_init(
                size,
                colorType,
                sampling);
            return out;
        }

        const imaging::Size& OffscreenBuffer::getSize() const
        {
            return _size;
        }

        imaging::PixelType OffscreenBuffer::getColorType() const
        {
            return _colorType;
        }

        OffscreenSampling OffscreenBuffer::getSampling() const
        {
            return _sampling;
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
