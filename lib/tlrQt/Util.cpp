// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/Util.h>

#include <tlrQt/TimeObject.h>

void qtInitResources()
{
    Q_INIT_RESOURCE(tlrQt);
}

namespace tlr
{
    namespace qt
    {
        void init()
        {
            qtInitResources();

            qRegisterMetaType<TimeUnits>("tlr::qt::TimeUnits");
            qRegisterMetaTypeStreamOperators<qt::TimeUnits>("tlr::qt::TimeUnits");
        }
    }
}

