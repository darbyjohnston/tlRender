// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDData.h>

#include <tlGL/GL.h>

#include <tlCore/Image.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include "DeckLinkAPI.h"

#include <string>

namespace tl
{
    namespace device
    {
        //! Convert to BMD.
        BMDPixelFormat toBMD(PixelType);

        //! Convert from BMD.
        PixelType fromBMD(BMDPixelFormat);

        //! Get a label.
        std::string getVideoConnectionLabel(BMDVideoConnection);

        //! Get a label.
        std::string getAudioConnectionLabel(BMDAudioConnection);

        //! Get a label.
        std::string getDisplayModeLabel(BMDDisplayMode);

        //! Get a label.
        std::string getPixelFormatLabel(BMDPixelFormat);

        //! Get a label.
        std::string getOutputFrameCompletionResultLabel(BMDOutputFrameCompletionResult);

        //! Get the output pixel type.
        device::PixelType getOutputType(device::PixelType);

        //! Get the offscreen buffer type.
        image::PixelType getOffscreenType(device::PixelType);

        //! Get the pack pixels buffer size.
        size_t getPackPixelsSize(const math::Size2i&, device::PixelType);

        //! Get the pack pixels format.
        GLenum getPackPixelsFormat(device::PixelType);

        //! Get the pack pixels type.
        GLenum getPackPixelsType(device::PixelType);

        //! Get the pack pixels alignment.
        GLint getPackPixelsAlign(device::PixelType);

        //! Get the pack pixels endian byte swap.
        GLint getPackPixelsSwap(device::PixelType);

        //! Copy the pack pixels.
        void copyPackPixels(
            const void*,
            void*,
            const math::Size2i&,
            device::PixelType);
    }
}
