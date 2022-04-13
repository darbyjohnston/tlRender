// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/ToolWidget.h>

namespace tl
{
    namespace play
    {
        //! Output widget.
        class OutputWidget : public QWidget
        {
            Q_OBJECT

        public:
            OutputWidget(QWidget* parent = nullptr);

            ~OutputWidget() override;

        public Q_SLOTS:

        Q_SIGNALS:

        private:
            TLRENDER_PRIVATE();
        };

        //! Output tool.
        class OutputTool : public ToolWidget
        {
            Q_OBJECT

        public:
            OutputTool(QWidget* parent = nullptr);

            ~OutputTool() override;

        public Q_SLOTS:

        Q_SIGNALS:

        private:
            TLRENDER_PRIVATE();
        };
    }
}
