// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/ToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Files tool.
        class FilesTool : public ToolWidget
        {
            Q_OBJECT

        public:
            FilesTool(
                const QMap<QString, QAction*>&,
                App*,
                QWidget* parent = nullptr);

            ~FilesTool() override;

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
