// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGlad/gl.h>

#include <QPalette>

namespace tl
{
    //! Qt QWidget support.
    namespace qwidget
    {
        //! Initialize the library. This needs to be called before the Qt application is instantiated.
        void init();

        //! Get a dark style color palette.
        QPalette darkStyle();

        //! Get a style sheet to fix the dock widget icons.
        QString dockWidgetStyleSheet();
    }
}

