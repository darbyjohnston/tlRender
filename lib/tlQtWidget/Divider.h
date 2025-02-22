// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <QFrame>

namespace tl
{
    namespace qtwidget
    {
        //! Divider.
        class Divider : public QFrame
        {
            Q_OBJECT

        public:
            Divider(Qt::Orientation, QWidget* parent = nullptr);

            virtual ~Divider();

            //! Set the orientation.
            void setOrientation(Qt::Orientation);

        private:
            void _widgetUpdate();

            Qt::Orientation _orientation = Qt::Horizontal;
        };
    }
}
