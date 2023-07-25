// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlQt/MetaTypes.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Compare tool.
        class CompareTool : public IToolWidget
        {
            Q_OBJECT

        public:
            CompareTool(
                const QMap<QString, QAction*>&,
                App*,
                QWidget* parent = nullptr);

            ~CompareTool() override;

        private Q_SLOTS:
            void _activatedCallback(const QModelIndex&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };

        //! Compare tool dock widget.
        class CompareDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            CompareDockWidget(
                CompareTool*,
                QWidget* parent = nullptr);
        };
    }
}
