// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/Util.h>

#include <QObject>

namespace tl
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
