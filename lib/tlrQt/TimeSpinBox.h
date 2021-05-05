// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Timeline.h>

#include <QAbstractSpinBox>

namespace tlr
{
    namespace qt
    {
        //! Time spin box.
        class TimeSpinBox : public QAbstractSpinBox
        {
            Q_OBJECT

        public:
            TimeSpinBox(QWidget* parent = nullptr);

            void fixup(QString&) const override;
            void stepBy(int steps) override;
            QValidator::State validate(QString&, int& pos) const override;
            QSize minimumSizeHint() const override;

        public Q_SLOTS:
            //! Set the value.
            void setValue(const otime::RationalTime&);
            
        Q_SIGNALS:
            //! This signal is emitted when the time is changed.
            void valueChanged(const otime::RationalTime&);

        protected:
            QAbstractSpinBox::StepEnabled stepEnabled() const override;

        private Q_SLOTS:
            void _lineEditCallback();

        private:
            void _textUpdate();

            otime::RationalTime _time;
        };
    }
}
