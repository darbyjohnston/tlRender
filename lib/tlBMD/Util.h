// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include "DeckLinkAPI.h"

#include <memory>
#include <string>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! Blackmagic Design DeckLink support.
    namespace bmd
    {
        //! Initialize the library.
        void init(const std::shared_ptr<system::Context>&);

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
