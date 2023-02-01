// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

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

        //! Initialize GLAD.
        void initGLAD();

        //! Get the glReadPixels format.
        unsigned int getReadPixelsFormat(imaging::PixelType);

        //! Get the glReadPixels type.
        unsigned int getReadPixelsType(imaging::PixelType);
    }
}
