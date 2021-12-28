// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrGL/Render.h>

#include <tlrCore/FontSystem.h>
#include <tlrCore/Image.h>

namespace tlr
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
        const std::shared_ptr<gl::Render>&,
        const std::shared_ptr<imaging::FontSystem>&,
        const imaging::Size& window,
        const std::string& text,
        imaging::FontFamily,
        uint16_t fontSize,
        HUDElement);
}
