// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/Texture.h>

#include <tlCore/Assert.h>

#include <array>
#include <iostream>

namespace tl
{
    namespace gl
    {
        GLenum getTextureFormat(imaging::PixelType value)
        {
            const std::array<GLenum, static_cast<std::size_t>(imaging::PixelType::Count)> data =
            {
                GL_NONE,

                GL_RED,
                GL_RED,
                GL_RED,
                GL_RED,
                GL_RED,

                GL_RG,
                GL_RG,
                GL_RG,
                GL_RG,
                GL_RG,

                GL_RGB,
                GL_RGBA,
                GL_RGB,
                GL_RGB,
                GL_RGB,
                GL_RGB,

                GL_RGBA,
                GL_RGBA,
                GL_RGBA,
                GL_RGBA,
                GL_RGBA,

                GL_NONE
            };
            return data[static_cast<std::size_t>(value)];
        }

        GLenum getTextureInternalFormat(imaging::PixelType type)
        {
            const std::array<GLenum, static_cast<std::size_t>(imaging::PixelType::Count)> data =
            {
                GL_NONE,

                GL_R8,
                GL_R16,
                GL_R32I,
                GL_R16F,
                GL_R32F,

                GL_RG8,
                GL_RG16,
                GL_RG32I,
                GL_RG16F,
                GL_RG32F,

                GL_RGB8,
                GL_RGB10,
                GL_RGB16,
                GL_RGB32I,
                GL_RGB16F,
                GL_RGB32F,

                GL_RGBA8,
                GL_RGBA16,
                GL_RGBA32I,
                GL_RGBA16F,
                GL_RGBA32F,

                GL_NONE
            };
            return data[static_cast<std::size_t>(type)];
        }

        GLenum getTextureType(imaging::PixelType value)
        {
            const std::array<GLenum, static_cast<std::size_t>(imaging::PixelType::Count)> data =
            {
                GL_NONE,

                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_SHORT,
                GL_UNSIGNED_INT,
                GL_HALF_FLOAT,
                GL_FLOAT,

                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_SHORT,
                GL_UNSIGNED_INT,
                GL_HALF_FLOAT,
                GL_FLOAT,

                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_INT_10_10_10_2,
                GL_UNSIGNED_SHORT,
                GL_UNSIGNED_INT,
                GL_HALF_FLOAT,
                GL_FLOAT,

                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_SHORT,
                GL_UNSIGNED_INT,
                GL_HALF_FLOAT,
                GL_FLOAT,

                GL_NONE
            };
            return data[static_cast<std::size_t>(value)];
        }

        void Texture::_init(const imaging::Info& info, const TextureOptions& options)
        {
            _info = info;
            if (_info.isValid())
            {
                if (options.pbo &&
                    1 == _info.layout.alignment &&
                    memory::getEndian() == _info.layout.endian)
                {
                    glGenBuffers(1, &_pbo);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pbo);
                    glBufferData(
                        GL_PIXEL_UNPACK_BUFFER,
                        imaging::getDataByteCount(_info),
                        NULL,
                        GL_STREAM_DRAW);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                }
                glGenTextures(1, &_id);
                glBindTexture(GL_TEXTURE_2D, _id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.filterMin);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.filterMag);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    getTextureInternalFormat(_info.pixelType),
                    _info.size.w,
                    _info.size.h,
                    0,
                    getTextureFormat(_info.pixelType),
                    getTextureType(_info.pixelType),
                    NULL);
            }
        }

        Texture::Texture()
        {}

        Texture::~Texture()
        {
            if (_pbo)
            {
                glDeleteBuffers(1, &_pbo);
                _pbo = 0;
            }
            if (_id)
            {
                glDeleteTextures(1, &_id);
                _id = 0;
            }
        }

        std::shared_ptr<Texture> Texture::create(const imaging::Info& info, const TextureOptions& options)
        {
            auto out = std::shared_ptr<Texture>(new Texture);
            out->_init(info, options);
            return out;
        }

        const imaging::Info& Texture::getInfo() const
        {
            return _info;
        }

        GLuint Texture::getID() const
        {
            return _id;
        }

        void Texture::copy(const imaging::Image& data)
        {
            if (_pbo)
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pbo);
                if (void* buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                {
                    memcpy(
                        buffer,
                        data.getData(),
                        data.getDataByteCount());
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                    const auto& info = data.getInfo();
                    glBindTexture(GL_TEXTURE_2D, _id);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                    glPixelStorei(GL_UNPACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                    glTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        0,
                        0,
                        info.size.w,
                        info.size.h,
                        getTextureFormat(info.pixelType),
                        getTextureType(info.pixelType),
                        NULL);
                }
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
            {
                const auto& info = data.getInfo();
                glBindTexture(GL_TEXTURE_2D, _id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    info.size.w,
                    info.size.h,
                    getTextureFormat(info.pixelType),
                    getTextureType(info.pixelType),
                    data.getData());
            }
        }

        void Texture::copy(const uint8_t* data, const imaging::Info& info)
        {
            if (_pbo)
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pbo);
                if (void* buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                {
                    memcpy(
                        buffer,
                        data,
                        imaging::getDataByteCount(info));
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                    glBindTexture(GL_TEXTURE_2D, _id);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                    glPixelStorei(GL_UNPACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                    glTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        0,
                        0,
                        info.size.w,
                        info.size.h,
                        getTextureFormat(info.pixelType),
                        getTextureType(info.pixelType),
                        NULL);
                }
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, _id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    info.size.w,
                    info.size.h,
                    getTextureFormat(info.pixelType),
                    getTextureType(info.pixelType),
                    data);
            }
        }

        void Texture::copy(const imaging::Image& data, uint16_t x, uint16_t y)
        {
            if (_pbo)
            {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _pbo);
                if (void* buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY))
                {
                    memcpy(
                        buffer,
                        data.getData(),
                        data.getDataByteCount());
                    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                    const auto& info = data.getInfo();
                    glBindTexture(GL_TEXTURE_2D, _id);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                    glPixelStorei(GL_UNPACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                    glTexSubImage2D(
                        GL_TEXTURE_2D,
                        0,
                        x,
                        y,
                        info.size.w,
                        info.size.h,
                        getTextureFormat(info.pixelType),
                        getTextureType(info.pixelType),
                        NULL);
                }
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
            else
            {
                const auto& info = data.getInfo();
                glBindTexture(GL_TEXTURE_2D, _id);
                glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
                glPixelStorei(GL_UNPACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    x,
                    y,
                    info.size.w,
                    info.size.h,
                    getTextureFormat(info.pixelType),
                    getTextureType(info.pixelType),
                    data.getData());
            }
        }

        void Texture::bind()
        {
            glBindTexture(GL_TEXTURE_2D, _id);
        }
    }
}
