// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>
#include <tlCore/Time.h>

#include "DeckLinkAPI.h"

#include <future>

namespace tl
{
    namespace bmd
    {
        //! Display mode.
        struct DisplayMode
        {
            BMDDisplayMode displayMode = BMDDisplayMode::bmdModeUnknown;
            imaging::Size size;
            otime::RationalTime frameRate;

            bool operator == (const DisplayMode&) const;
        };

        //! Device information.
        struct DeviceInfo
        {
            std::string model;
            std::vector<DisplayMode> displayModes;

            bool operator == (const DeviceInfo&) const;
        };

        //! Get device information.
        std::future<std::vector<DeviceInfo> > getInfo();
    }
}
