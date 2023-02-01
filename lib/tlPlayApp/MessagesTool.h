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
        //! Messages tool.
        class MessagesTool : public ToolWidget
        {
            Q_OBJECT

        public:
            MessagesTool(
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            ~MessagesTool() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! Messages tool dock widget.
        class MessagesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            MessagesDockWidget(
                MessagesTool*,
                QWidget* parent = nullptr);
        };
    }
}
