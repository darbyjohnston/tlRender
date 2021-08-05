// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>

#include <QWidget>

#include <memory>

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

            ~TimeLabel() override;
            
            //! Set the time object.
            void setTimeObject(TimeObject*);

        public Q_SLOTS:
            //! Set the time value.
            void setValue(const otime::RationalTime&);
            
            //! Set the time units.
            void setUnits(qt::TimeUnits);

        private:
            void _textUpdate();

            TLR_PRIVATE();
        };
    }
}
