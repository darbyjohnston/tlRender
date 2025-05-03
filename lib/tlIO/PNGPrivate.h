// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

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
