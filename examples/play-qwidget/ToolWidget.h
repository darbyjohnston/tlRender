// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <QVBoxLayout>
#include <QWidget>

namespace tl
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
