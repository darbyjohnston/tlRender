// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrQt/Util.h>

#include <QObject>

namespace tlr
{
    namespace qt
    {
        //! Tool tip filter.
        class ToolTipsFilter : public QObject
        {
            Q_OBJECT

        public:
            ToolTipsFilter(QObject* parent = nullptr);

            bool eventFilter(QObject*, QEvent*) override;
        };
    }
}
