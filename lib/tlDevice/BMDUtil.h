// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDDeviceData.h>

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

        //! Get the offscreen buffer type.
        image::PixelType getOffscreenType(device::PixelType);

        //! Get the reead pixels format.
        GLenum getReadPixelsFormat(device::PixelType);

        //! Get the reead pixels type.
        GLenum getReadPixelsType(device::PixelType);

        //! Get the reead pixels alignment.
        GLint getReadPixelsAlign(device::PixelType);

        //! Get the reead pixels endian byte swap.
        GLint getReadPixelsSwap(device::PixelType);
    }
}
