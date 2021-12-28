// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Image.h>

#include <tlrGlad/gl.h>

namespace tlr
{
    //! OpenGL support.
    namespace gl
    {
        //! Get the glReadPixels format.
        GLenum getReadPixelsFormat(imaging::PixelType);

        //! Get the glReadPixels type.
        GLenum getReadPixelsType(imaging::PixelType);
    }
}
