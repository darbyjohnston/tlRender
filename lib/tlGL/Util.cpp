// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/Util.h>

#include <tlTimeline/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>

#include <tlGlad/gl.h>

#include <array>
#include <iostream>

namespace tl
{
    namespace gl
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            timeline::init(context);
        }

        void initGLAD()
        {
            gladLoaderLoadGL();
        }

        unsigned int getReadPixelsFormat(imaging::PixelType value)
        {
            const std::array<GLenum, static_cast<std::size_t>(imaging::PixelType::Count)> data =
            {
                GL_NONE,

                GL_RED,
                GL_RED,
                GL_RED,
                GL_RED,
                GL_RED,

                GL_NONE,
                GL_NONE,
                GL_NONE,
                GL_NONE,
                GL_NONE,

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

        unsigned int getReadPixelsType(imaging::PixelType value)
        {
            const std::array<GLenum, static_cast<std::size_t>(imaging::PixelType::Count)> data =
            {
                GL_NONE,

                GL_UNSIGNED_BYTE,
                GL_UNSIGNED_SHORT,
                GL_UNSIGNED_INT,
                GL_HALF_FLOAT,
                GL_FLOAT,

                GL_NONE,
                GL_NONE,
                GL_NONE,
                GL_NONE,
                GL_NONE,

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
    }
}
