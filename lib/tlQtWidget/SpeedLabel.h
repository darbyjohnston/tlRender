// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Speed label.
        class SpeedLabel : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                OTIO_NS::RationalTime value
                READ value
                WRITE setValue)

        public:
            SpeedLabel(QWidget* parent = nullptr);

            virtual ~SpeedLabel();

            //! Get the speed value.
            const OTIO_NS::RationalTime& value() const;

        public Q_SLOTS:
            //! Set the speed value.
            void setValue(const OTIO_NS::RationalTime&);

        private:
            void _textUpdate();

            DTK_PRIVATE();
        };
    }
}
