// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        //! System log tool.
        class SystemLogTool : public IToolWidget
        {
            Q_OBJECT

        public:
            SystemLogTool(App*, QWidget* parent = nullptr);

            virtual ~SystemLogTool();

        private:
            DTK_PRIVATE();
        };

        //! System log tool dock widget.
        class SystemLogDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            SystemLogDockWidget(SystemLogTool*, QWidget* parent = nullptr);
        };
    }
}
