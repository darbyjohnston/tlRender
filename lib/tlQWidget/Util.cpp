// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQWidget/Util.h>

#include <tlQt/Util.h>

#include <QDir>

void qtInitResources()
{
    Q_INIT_RESOURCE(tlQWidget);
}

namespace tl
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

