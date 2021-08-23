// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>

#include <QAbstractSpinBox>

#include <memory>

namespace tlr
{
    namespace qwidget
    {
        //! Time spin box.
        class TimeSpinBox : public QAbstractSpinBox
        {
            Q_OBJECT

        public:
            TimeSpinBox(QWidget* parent = nullptr);
            
            ~TimeSpinBox() override;

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Get the time value.
            const otime::RationalTime& value() const;

            //! Get the time units.
            qt::TimeUnits units() const;

            void stepBy(int steps) override;
            QValidator::State validate(QString&, int& pos) const override;
            QSize minimumSizeHint() const override;

        public Q_SLOTS:
            //! Set the time value.
            void setValue(const otime::RationalTime&);
            
            //! Set the time units.
            void setUnits(tlr::qt::TimeUnits);

        Q_SIGNALS:
            //! This signal is emitted when the time is changed.
            void valueChanged(const otime::RationalTime&);

            //! This signal is emitted when the time units are changed.
            void unitsChanged(tlr::qt::TimeUnits);

        protected:
            QAbstractSpinBox::StepEnabled stepEnabled() const override;

        private Q_SLOTS:
            void _lineEditCallback();

        private:
            void _vaidatorUpdate();
            void _textUpdate();

            TLR_PRIVATE();
        };
    }
}
