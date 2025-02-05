// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidgetOptions.h>

#include <dtk/core/Box.h>

namespace tl
{
    namespace ui
    {
        //! Align within the given box.
        dtk::Box2I align(
            const dtk::Box2I&  box,
            const dtk::Size2I& sizeHint,
            Stretch            hStretch,
            Stretch            vStretch,
            HAlign             hAlign,
            VAlign             vAlign);

        //! Get a format string for the given number.
        std::string format(int);

        //! Get a format string for the given number.
        std::string format(float, int precision);
    }
}
