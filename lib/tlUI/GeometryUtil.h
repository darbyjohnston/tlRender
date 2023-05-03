// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidgetOptions.h>

#include <tlCore/BBox.h>

namespace tl
{
    namespace ui
    {
        //! Align within the given bounding box.
        math::BBox2i align(
            const math::BBox2i&   bbox,
            const math::Vector2i& sizeHint,
            Stretch               hStretch,
            Stretch               vStretch,
            HAlign                hAlign,
            VAlign                vAlign);

        //! Get a format string for the given number.
        std::string format(int);

        //! Get a format string for the given number.
        std::string format(float, int precision);
    }
}
