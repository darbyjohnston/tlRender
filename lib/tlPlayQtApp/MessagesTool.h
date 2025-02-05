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
        //! Messages tool.
        class MessagesTool : public IToolWidget
        {
            Q_OBJECT

        public:
            MessagesTool(App*, QWidget* parent = nullptr);

            virtual ~MessagesTool();

        private:
            DTK_PRIVATE();
        };

        //! Messages tool dock widget.
        class MessagesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            MessagesDockWidget(MessagesTool*, QWidget* parent = nullptr);
        };
    }
}
