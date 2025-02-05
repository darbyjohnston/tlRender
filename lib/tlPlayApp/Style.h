// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Style.h>

namespace tl
{
    namespace play_app
    {
        //! Style palettes.
        enum class StylePalette
        {
            Dark,
            Light,

            Count,
            First = Dark
        };
        DTK_ENUM(StylePalette);

        //! Get the style palette.
        std::map<ui::ColorRole, dtk::Color4F> getStylePalette(StylePalette);
    }
}
