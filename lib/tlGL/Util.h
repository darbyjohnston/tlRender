// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Image.h>

namespace tl
{
    namespace gl
    {
        //! Get the glReadPixels format.
        unsigned int getReadPixelsFormat(imaging::PixelType);

        //! Get the glReadPixels type.
        unsigned int getReadPixelsType(imaging::PixelType);

        //! Set whether an OpenGL capability is enabled and restore it to the
        //! previous value when finished.
        class SetAndRestore
        {
        public:
            SetAndRestore(unsigned int, bool);

            ~SetAndRestore();

        private:
            TLRENDER_PRIVATE();
        };
    }
}
