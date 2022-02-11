// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <QAbstractButton>
#include <QVariant>

namespace tlr
{
    namespace qwidget
    {
        //! Radio button group.
        class RadioButtonGroup : public QWidget
        {
            Q_OBJECT

        public:
            RadioButtonGroup(Qt::Orientation = Qt::Horizontal, QWidget* parent = nullptr);

            ~RadioButtonGroup() override;
            
            //! Add a button.
            void addButton(const QString&, const QVariant&);

            //! Clear the buttons.
            void clear();

        public Q_SLOTS:
            //! Set the checked radio button.
            void setChecked(const QVariant&);

            //! Set the orientation.
            void setOrientation(Qt::Orientation);

        Q_SIGNALS:
            //! This signal is emitted when a radio button is checked.
            void checked(const QVariant&);

        private Q_SLOTS:
            void _groupCallback(QAbstractButton* button, bool);

        private:
            void _widgetUpdate();

            TLR_PRIVATE();
        };
    }
}
