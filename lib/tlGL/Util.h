// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <tlGlad/gl.h>

namespace tl
{
    //! OpenGL renderer.
    namespace gl
    {
        //! Get the glReadPixels format.
        GLenum getReadPixelsFormat(imaging::PixelType);

        //! Get the glReadPixels type.
        GLenum getReadPixelsType(imaging::PixelType);
    }
}
