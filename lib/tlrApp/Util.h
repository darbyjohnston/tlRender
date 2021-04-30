// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrRender/Render.h>

#include <tlrAV/Image.h>

#include <tlrCore/BBox.h>

namespace tlr
{
    namespace app
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
            const std::shared_ptr<render::Render>&,
            const std::shared_ptr<render::FontSystem>&,
            const imaging::Size& window,
            const std::string& text,
            render::FontFamily,
            uint16_t fontSize,
            HUDElement);
    }
}
