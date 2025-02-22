// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Divider.h>

namespace tl
{
    namespace qtwidget
    {
        Divider::Divider(Qt::Orientation orientation, QWidget* parent) :
            QFrame(parent),
            _orientation(orientation)
        {
            setForegroundRole(QPalette::Mid);
            _widgetUpdate();
        }

        Divider::~Divider()
        {}

        void Divider::setOrientation(Qt::Orientation value)
        {
            if (value == _orientation)
                return;
            _orientation = value;
            _widgetUpdate();
        }

        void Divider::_widgetUpdate()
        {
            setFrameShape(Qt::Horizontal ? QFrame::HLine : QFrame::VLine);
        }
    }
}
