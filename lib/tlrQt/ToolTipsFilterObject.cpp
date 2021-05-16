// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/ToolTipsFilterObject.h>

#include <QEvent>

namespace tlr
{
    namespace qt
    {
        ToolTipsFilterObject::ToolTipsFilterObject(QObject* parent) :
            QObject(parent)
        {}

        bool ToolTipsFilterObject::eventFilter(QObject* watched, QEvent* event)
        {
            if (event->type() == QEvent::Type::ToolTip)
            {
                return true;
            }
            return false;
        }
    }
}
