// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQuick/Util.h>

#include <tlrQuick/GLFramebufferObject.h>

#include <tlrQt/Util.h>

namespace tlr
{
    namespace quick
    {
        void init()
        {
            qt::init();
            
            qmlRegisterType<GLFramebufferObject>("tlrQuick", 1, 0, "GLFramebufferObject");

            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }
    }
}

