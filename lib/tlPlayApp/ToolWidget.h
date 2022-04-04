// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QVBoxLayout>
#include <QWidget>

#include <memory>

namespace tl
{
    namespace play
    {
        //! Base class for tool widgets.
        class ToolWidget : public QWidget
        {
            Q_OBJECT

        public:
            ToolWidget(QWidget* parent = nullptr);

            void addWidget(QWidget*, int stretch = 0);
            void addBellows(const QString&, QWidget*);
            void addStretch(int stretch = 0);

        private:
            QVBoxLayout* _layout = nullptr;
        };
    }
}
