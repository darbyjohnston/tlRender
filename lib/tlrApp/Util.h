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
        math::BBox2f fitImageInWindow(const imaging::Size& image, const imaging::Size& window);

        enum class HUDElement
        {
            UpperLeft,
            UpperRight,
            LowerLeft,
            LowerRight
        };

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
