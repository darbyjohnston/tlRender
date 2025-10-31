// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

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
