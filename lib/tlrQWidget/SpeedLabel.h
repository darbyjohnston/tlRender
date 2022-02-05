// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/TimeObject.h>

#include <QWidget>

#include <memory>

namespace tlr
{
    namespace qwidget
    {
        //! Speed label.
        class SpeedLabel : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                otime::RationalTime value
                READ value
                WRITE setValue)

        public:
            SpeedLabel(QWidget* parent = nullptr);
            
            ~SpeedLabel() override;

            //! Get the speed value.
            const otime::RationalTime& value() const;

        public Q_SLOTS:
            //! Set the speed value.
            void setValue(const otime::RationalTime&);
            
        private:
            void _textUpdate();

            TLR_PRIVATE();
        };
    }
}
