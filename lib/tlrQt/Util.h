// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <glad/gl.h>

namespace tlr
{
    //! Qt support.
    namespace qt
    {
        //! Initialize Qt. This needs to be called before the QApplication is instantiated.
        void init();
    }
}

