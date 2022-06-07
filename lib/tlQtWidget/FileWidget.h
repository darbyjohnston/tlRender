// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! File widget.
        class FileWidget : public QWidget
        {
            Q_OBJECT

        public:
            FileWidget(QWidget* parent = nullptr);

            ~FileWidget() override;

        public Q_SLOTS:
            //! Set the file.
            void setFile(const QString&);

            //! Clear the file.
            void clear();

        Q_SIGNALS:
            //! This signal is emitted when the file is changed.
            void fileChanged(const QString&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
