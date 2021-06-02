// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>

#include <tlrCore/Timeline.h>

#include <QLabel>
#include <QPointer>

namespace tlr
{
    namespace qt
    {
        //! Speed label.
        class SpeedLabel : public QWidget
        {
            Q_OBJECT

        public:
            SpeedLabel(QWidget* parent = nullptr);

        public Q_SLOTS:
            //! Set the speed value.
            void setValue(const otime::RationalTime&);
            
        private:
            void _textUpdate();

            otime::RationalTime _value = invalidTime;
            QLabel* _label = nullptr;
        };
    }
}
