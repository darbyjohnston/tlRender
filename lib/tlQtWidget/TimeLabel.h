// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Time label.
        class TimeLabel : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                otime::RationalTime value
                READ value
                WRITE setValue)
            Q_PROPERTY(
                tl::qt::TimeUnits units
                READ units
                WRITE setUnits)

        public:
            TimeLabel(QWidget* parent = nullptr);

            ~TimeLabel() override;

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Get the time value.
            const otime::RationalTime& value() const;

            //! Get the time units.
            qt::TimeUnits units() const;

        public Q_SLOTS:
            //! Set the time value.
            void setValue(const otime::RationalTime&);

            //! Set the time units.
            void setUnits(tl::qt::TimeUnits);

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
