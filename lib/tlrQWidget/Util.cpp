// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQWidget/Util.h>

#include <tlrQt/Util.h>

#include <QDir>

void qtInitResources()
{
    Q_INIT_RESOURCE(tlrQWidget);
}

namespace tlr
{
    namespace qwidget
    {
        void init()
        {
            qt::init();

            qtInitResources();
        }
    }
}

