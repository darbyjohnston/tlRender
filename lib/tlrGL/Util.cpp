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
        GLenum getReadPixelsFormat(imaging::PixelType value)
        {
            const std::array<GLenum, static_cast<std::size_t>(imaging::PixelType::Count)> data =
            {
                GL_NONE,
                GL_RED,
                GL_RED,
                GL_RED,
                GL_NONE,
                GL_NONE,
                GL_NONE,
                GL_RGB,
                GL_RGB,
                GL_RGB,
                GL_RGBA,
                GL_RGBA,
                GL_RGBA,
                GL_RGBA,
                GL_NONE
            };
            return data[static_cast<std::size_t>(value)];
        }

        GLenum getReadPixelsType(imaging::PixelType value)
        {
            const std::array<GLenum, static_cast<std::size_t>(imaging::PixelType::Count)> data =
            {
                GL_NONE,
                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_SHORT,
                GL_FLOAT,
                GL_NONE,
                GL_NONE,
                GL_NONE,
                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_SHORT,
                GL_FLOAT,
                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_SHORT,
                GL_HALF_FLOAT,
                GL_FLOAT,
                GL_NONE
            };
            return data[static_cast<std::size_t>(value)];
        }
    }
}
