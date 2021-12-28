// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrGlad/gl.h>

namespace tlr
{
    //! Qt QWidget support.
    namespace qwidget
    {
        //! Initialize the library. This needs to be called before the Qt application is instantiated.
        void init();
    }
}

