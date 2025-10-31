// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/Box.h>

namespace tl
{
    namespace timeline
    {
        //! Get a box with the given aspect ratio that fits within
        //! the given box.
        ftk::Box2I getBox(float aspect, const ftk::Box2I&);
    }
}
