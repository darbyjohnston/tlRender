// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Style.h>

namespace tl
{
    namespace play_gl
    {
        //! Style palettes.
        enum class StylePalette
        {
            Dark,
            Light,

            Count,
            First = Dark
        };
        TLRENDER_ENUM(StylePalette);
        TLRENDER_ENUM_SERIALIZE(StylePalette);

        //! Get the style palette.
        std::map<ui::ColorRole, imaging::Color4f> getStylePalette(StylePalette);
    }
}
