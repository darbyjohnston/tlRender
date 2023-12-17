// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQt/BMDOutputDevicePrivate.h>

namespace tl
{
    namespace qt
    {
        image::PixelType getOffscreenType(device::PixelType value)
        {
            const std::array<image::PixelType, static_cast<size_t>(device::PixelType::Count)> data =
            {
                image::PixelType::None,
                image::PixelType::RGBA_U8,
                image::PixelType::RGB_U10
            };
            return data[static_cast<size_t>(value)];
        }

        GLenum getReadPixelsFormat(device::PixelType value)
        {
            const std::array<GLenum, static_cast<size_t>(device::PixelType::Count)> data =
            {
                GL_NONE,
                GL_BGRA,
                GL_RGBA
            };
            return data[static_cast<size_t>(value)];
        }

        GLenum getReadPixelsType(device::PixelType value)
        {
            const std::array<GLenum, static_cast<size_t>(device::PixelType::Count)> data =
            {
                GL_NONE,
                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_INT_10_10_10_2
            };
            return data[static_cast<size_t>(value)];
        }

        GLint getReadPixelsAlign(device::PixelType value)
        {
            const std::array<GLint, static_cast<size_t>(device::PixelType::Count)> data =
            {
                0,
                4,
                256
            };
            return data[static_cast<size_t>(value)];
        }

        GLint getReadPixelsSwap(device::PixelType value)
        {
            const std::array<GLint, static_cast<size_t>(device::PixelType::Count)> data =
            {
                GL_FALSE,
                GL_FALSE,
                GL_FALSE
            };
            return data[static_cast<size_t>(value)];
        }

        OverlayTexture::OverlayTexture(const QSize& size, QImage::Format format) :
            _size(size),
            _format(format)
        {
            switch (format)
            {
            case QImage::Format_RGBA8888:
                _textureFormat = GL_RGBA;
                _textureType = GL_UNSIGNED_BYTE;
                break;
            case QImage::Format_ARGB4444_Premultiplied:
                _textureFormat = GL_BGRA;
                _textureType = GL_UNSIGNED_SHORT_4_4_4_4_REV;
                break;
            default: break;
            }
            if (_textureFormat != GL_NONE && _textureType != GL_NONE)
            {
                glGenTextures(1, &_id);
                glBindTexture(GL_TEXTURE_2D, _id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RGBA8,
                    _size.width(),
                    _size.height(),
                    0,
                    _textureFormat,
                    _textureType,
                    NULL);
            }
        }

        OverlayTexture::~OverlayTexture()
        {
            if (_id)
            {
                glDeleteTextures(1, &_id);
                _id = 0;
            }
        }

        std::shared_ptr<OverlayTexture> OverlayTexture::create(const QSize& size, QImage::Format format)
        {
            return std::shared_ptr<OverlayTexture>(new OverlayTexture(size, format));
        }

        void OverlayTexture::copy(const QImage& value)
        {
            if (value.size() == _size && value.format() == _format)
            {
                glBindTexture(GL_TEXTURE_2D, _id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, 0);
                glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    _size.width(),
                    _size.height(),
                    _textureFormat,
                    _textureType,
                    value.bits());
            }
        }
    }
}
