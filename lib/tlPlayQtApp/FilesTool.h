// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Files tool.
        class FilesTool : public IToolWidget
        {
            Q_OBJECT

        public:
            FilesTool(
                const QMap<QString, QAction*>&,
                App*,
                QWidget* parent = nullptr);

            virtual ~FilesTool();

        private Q_SLOTS:
            void _activatedCallback(const QModelIndex&);

        private:
            TLRENDER_PRIVATE();
        };

        //! Files tool dock widget.
        class FilesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            FilesDockWidget(
                FilesTool*,
                QWidget* parent = nullptr);
        };
    }
}
