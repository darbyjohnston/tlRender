// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/ToolWidget.h>

#include <tlQt/MetaTypes.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Devices tool.
        class DevicesTool : public ToolWidget
        {
            Q_OBJECT

        public:
            DevicesTool(
                App*,
                QWidget* parent = nullptr);

            ~DevicesTool() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Devices tool dock widget.
        class DevicesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            DevicesDockWidget(
                DevicesTool*,
                QWidget* parent = nullptr);
        };
    }
}
