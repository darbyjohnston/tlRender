// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include "Render.h"

#include <tlrRender/BBox.h>
#include <tlrRender/Image.h>

namespace tlr
{
    //! Fit an image within a window.
    math::BBox2f fitImageInWindow(const imaging::Size& image, const imaging::Size& window);

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
        const std::shared_ptr<Render>&,
        const std::shared_ptr<FontSystem>&,
        const imaging::Size& window,
        const std::string& text,
        FontFamily,
        uint16_t fontSize,
        HUDElement);
}
