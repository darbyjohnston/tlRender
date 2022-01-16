// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <QToolBox>

namespace tlr
{
    //! Layers tool.
    class LayersTool : public QToolBox
    {
        Q_OBJECT

    public:
        LayersTool(QWidget* parent = nullptr);
    private Q_SLOTS:
        void _currentItemCallback(int);

    private:
    };
}
