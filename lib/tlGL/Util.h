// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

#include <tlGlad/gl.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! OpenGL renderer.
    namespace gl
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);

        //! Get the OpenGL texture filter.
        GLenum getTextureFilter(timeline::ImageFilter);

        //! Get the glReadPixels format.
        GLenum getReadPixelsFormat(imaging::PixelType);

        //! Get the glReadPixels type.
        GLenum getReadPixelsType(imaging::PixelType);
    }
}
