// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "CompareTool.h"

#include <QBoxLayout>
#include <QSettings>
#include <QSignalBlocker>

namespace tlr
{
    CompareTool::CompareTool(QWidget* parent) :
        QToolBox(parent)
    {
        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("CompareTool/CurrentItem").toInt());
    }

    void CompareTool::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("CompareTool/CurrentItem", value);
    }
}
