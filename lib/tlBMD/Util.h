// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "DeckLinkAPI.h"

#include <string>

namespace tl
{
    //! Blackmagic Design DeckLink support.
    namespace bmd
    {
        //! Get a label.
        std::string getLabel(BMDVideoConnection);

        //! Get a label.
        std::string getLabel(BMDAudioConnection);

        //! Get a label.
        std::string getLabel(BMDDisplayMode);

        //! Get a label.
        std::string getLabel(BMDPixelFormat);
    }
}
