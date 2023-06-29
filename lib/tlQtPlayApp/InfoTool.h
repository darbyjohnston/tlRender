// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtPlayApp/ToolWidget.h>

#include <QDockWidget>

namespace tl
{
    namespace io
    {
        struct Info;
    }

    namespace qtplay
    {
        class App;

        //! Information tool.
        class InfoTool : public ToolWidget
        {
            Q_OBJECT

        public:
            InfoTool(
                App*,
                QWidget* parent = nullptr);

            ~InfoTool() override;

            void setInfo(const io::Info&);

        private:
            TLRENDER_PRIVATE();
        };

        //! Information tool dock widget.
        class InfoDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            InfoDockWidget(
                InfoTool*,
                QWidget* parent = nullptr);
        };
    }
}
