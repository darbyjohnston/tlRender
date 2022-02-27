// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <tlGlad/gl.h>

namespace tl
{
    //! OpenGL functionality.
    namespace gl
    {
        //! Get the glReadPixels format.
        GLenum getReadPixelsFormat(core::imaging::PixelType);

        //! Get the glReadPixels type.
        GLenum getReadPixelsType(core::imaging::PixelType);
    }
}
