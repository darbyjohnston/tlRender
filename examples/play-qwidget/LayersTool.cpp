// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "LayersTool.h"

#include <QBoxLayout>
#include <QSettings>
#include <QSignalBlocker>

namespace tlr
{
    LayersTool::LayersTool(QWidget* parent) :
        QToolBox(parent)
    {
        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("LayersTool/CurrentItem").toInt());
    }

    void LayersTool::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("LayersTool/CurrentItem", value);
    }
}
