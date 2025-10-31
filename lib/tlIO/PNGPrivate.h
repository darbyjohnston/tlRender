// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/PNG.h>

#include <png.h>

namespace tl
{
    namespace png
    {
        //! PNG error.
        struct ErrorStruct
        {
            std::string message;
        };

        extern "C"
        {
            //! PNG error function.
            void errorFunc(png_structp in, png_const_charp msg);

            //! PNG warning function.
            void warningFunc(png_structp in, png_const_charp msg);
        }
    }
}
