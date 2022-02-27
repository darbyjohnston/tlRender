// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <QFrame>

namespace tl
{
    namespace qtwidget
    {
        //! Spacer.
        class Spacer : public QFrame
        {
            Q_OBJECT

        public:
            Spacer(Qt::Orientation, QWidget* parent = nullptr);

            ~Spacer() override;

            //! Set the orientation.
            void setOrientation(Qt::Orientation);

        private:
            void _widgetUpdate();

            Qt::Orientation _orientation = Qt::Horizontal;
        };
    }
}
