// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/Box.h>

namespace tl
{
    namespace timeline
    {
        //! Get a box with the given aspect ratio that fits within
        //! the given box.
        ftk::Box2I getBox(float aspect, const ftk::Box2I&);
    }
}
