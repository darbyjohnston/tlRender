// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/ToolTipsFilter.h>

#include <QEvent>

namespace tlr
{
    namespace qt
    {
        ToolTipsFilter::ToolTipsFilter(QObject* parent) :
            QObject(parent)
        {}

        bool ToolTipsFilter::eventFilter(QObject* watched, QEvent* event)
        {
            if (event->type() == QEvent::Type::ToolTip)
            {
                return true;
            }
            return false;
        }
    }
}
