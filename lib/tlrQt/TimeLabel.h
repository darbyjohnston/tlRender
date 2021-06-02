// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>

#include <tlrCore/Timeline.h>

#include <QLabel>

namespace tlr
{
    namespace qt
    {
        //! Time label.
        class TimeLabel : public QWidget
        {
            Q_OBJECT

        public:
            TimeLabel(QWidget* parent = nullptr);

            //! Set the time object.
            void setTimeObject(TimeObject*);

        public Q_SLOTS:
            //! Set the time value.
            void setValue(const otime::RationalTime&);
            
            //! Set the time units.
            void setUnits(qt::TimeObject::Units);

        private:
            void _textUpdate();

            otime::RationalTime _value = invalidTime;
            TimeObject::Units _units = TimeObject::Units::Timecode;
            QLabel* _label = nullptr;
            TimeObject* _timeObject = nullptr;
        };
    }
}
