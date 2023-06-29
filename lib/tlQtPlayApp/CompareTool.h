// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtPlayApp/ToolWidget.h>

#include <tlQt/MetaTypes.h>

#include <QDockWidget>

namespace tl
{
    namespace qtplay
    {
        class App;

        //! Compare tool.
        class CompareTool : public ToolWidget
        {
            Q_OBJECT

        public:
            CompareTool(
                const QMap<QString, QAction*>&,
                App*,
                QWidget* parent = nullptr);

            ~CompareTool() override;

        public Q_SLOTS:
            void setCompareOptions(const tl::timeline::CompareOptions&);

        private Q_SLOTS:
            void _activatedCallback(const QModelIndex&);

        Q_SIGNALS:
            void compareOptionsChanged(const tl::timeline::CompareOptions&);

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
