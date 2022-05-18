// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/DeviceData.h>

#include <tlCore/Image.h>

#include <tlGlad/gl.h>

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

        //! Get a label.
        std::string getLabel(BMDVideoConnection);

        //! Get a label.
        std::string getLabel(BMDAudioConnection);

        //! Get a label.
        std::string getLabel(BMDDisplayMode);

        //! Get a label.
        std::string getLabel(BMDPixelFormat);

        //! Get a label.
        std::string getLabel(BMDOutputFrameCompletionResult);
    }
}
