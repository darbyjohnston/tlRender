// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/ToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! System log tool.
        class SystemLogTool : public ToolWidget
        {
            Q_OBJECT

        public:
            SystemLogTool(
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            ~SystemLogTool() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! System log tool dock widget.
        class SystemLogDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            SystemLogDockWidget(
                SystemLogTool*,
                QWidget* parent = nullptr);
        };
    }
}
