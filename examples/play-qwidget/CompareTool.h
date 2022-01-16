// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <QToolBox>

namespace tlr
{
    //! Compare tool.
    class CompareTool : public QToolBox
    {
        Q_OBJECT

    public:
        CompareTool(QWidget* parent = nullptr);

    private Q_SLOTS:
        void _currentItemCallback(int);

    private:
    };
}
