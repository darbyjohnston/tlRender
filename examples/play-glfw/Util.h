// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/FontSystem.h>
#include <tlCore/IRender.h>
#include <tlCore/Image.h>

namespace tl
{
    //! HUD elements.
    enum class HUDElement
    {
        UpperLeft,
        UpperRight,
        LowerLeft,
        LowerRight
    };

    //! Draw a HUD label.
    void drawHUDLabel(
        const std::shared_ptr<render::IRender>&,
        const std::shared_ptr<imaging::FontSystem>&,
        const imaging::Size& window,
        const std::string& text,
        imaging::FontFamily,
        uint16_t fontSize,
        HUDElement);
}
