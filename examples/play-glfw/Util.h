// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <tlCore/FontSystem.h>
#include <tlCore/Image.h>

namespace tl
{
    //! Examples.
    namespace examples
    {
        namespace play_glfw
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
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<core::imaging::FontSystem>&,
                const core::imaging::Size& window,
                const std::string& text,
                core::imaging::FontFamily,
                uint16_t fontSize,
                HUDElement);
        }
    }
}
