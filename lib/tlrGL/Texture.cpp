// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrGL/Texture.h>

#include <tlrCore/Assert.h>

#include <array>
#include <iostream>

namespace tlr
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

        void Texture::_init(const imaging::Info& info, GLenum filterMin, GLenum filterMag)
        {
            _info = info;
            _filterMin = filterMin;
            _filterMag = filterMag;
            if (_info.isValid())
            {
                glGenTextures(1, &_id);
                glBindTexture(GL_TEXTURE_2D, _id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filterMin);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filterMag);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    getTextureInternalFormat(_info.pixelType),
                    _info.size.w,
                    _info.size.h,
                    0,
                    getTextureFormat(_info.pixelType),
                    getTextureType(_info.pixelType),
                    0);
            }
        }

        Texture::Texture()
        {}

        Texture::~Texture()
        {
            if (_id)
            {
                glDeleteTextures(1, &_id);
                _id = 0;
            }
        }

        std::shared_ptr<Texture> Texture::create(const imaging::Info& info, GLenum filterMin, GLenum filterMag)
        {
            auto out = std::shared_ptr<Texture>(new Texture);
            out->_init(info, filterMin, filterMag);
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

        void Texture::set(const imaging::Info& info)
        {
            if (info == _info)
                return;
            _info = info;
            if (_info.isValid())
            {
                if (_id)
                {
                    glDeleteTextures(1, &_id);
                }

                glGenTextures(1, &_id);
                glBindTexture(GL_TEXTURE_2D, _id);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filterMin);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filterMag);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    getTextureInternalFormat(_info.pixelType),
                    _info.size.w,
                    _info.size.h,
                    0,
                    getTextureFormat(_info.pixelType),
                    getTextureType(_info.pixelType),
                    0);
            }
        }

        void Texture::copy(const imaging::Image& data)
        {
            const auto& info = data.getInfo();
            glBindTexture(GL_TEXTURE_2D, _id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
            if (info.layout.endian != memory::getEndian())
            {
                glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
            }
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

        void Texture::copy(const uint8_t* data, const imaging::Info& info)
        {
            glBindTexture(GL_TEXTURE_2D, _id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
            if (info.layout.endian != memory::getEndian())
            {
                glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
            }
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

        void Texture::copy(const imaging::Image& data, uint16_t x, uint16_t y)
        {
            const auto& info = data.getInfo();
            glBindTexture(GL_TEXTURE_2D, _id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, info.layout.alignment);
            if (info.layout.endian != memory::getEndian())
            {
                glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
            }
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

        void Texture::bind()
        {
            glBindTexture(GL_TEXTURE_2D, _id);
        }
    }
}
