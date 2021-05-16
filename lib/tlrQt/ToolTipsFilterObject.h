// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <QObject>

namespace tlr
{
    namespace qt
    {
        //! Tool tip filter object.
        class ToolTipsFilterObject : public QObject
        {
            Q_OBJECT

        public:
            ToolTipsFilterObject(QObject* parent = nullptr);

            bool eventFilter(QObject*, QEvent*) override;
        };
    }
}
