// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <QFrame>

namespace tl
{
    namespace qwidget
    {
        //! Separator.
        class Separator : public QFrame
        {
            Q_OBJECT

        public:
            Separator(Qt::Orientation, QWidget* parent = nullptr);

            ~Separator() override;
            
            //! Set the orientation.
            void setOrientation(Qt::Orientation);

        private:
            void _widgetUpdate();

            Qt::Orientation _orientation = Qt::Orientation::Horizontal;
        };
    }
}
