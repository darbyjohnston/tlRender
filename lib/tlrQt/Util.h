// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrGlad/gl.h>

#include <QString>

namespace tlr
{
    //! Qt support.
    namespace qt
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is instantiated.
        void init();

        //! Create a settings key unique to the Qt version. This is for
        //! saving Qt window/widget state information which does not seem
        //! to be compatible between Qt 5 and 6.
        QString versionedSettingsKey(const QString&);
    }
}
