// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlAppPlay/ToolWidget.h>

#include <tlQt/MetaTypes.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Compare tool.
        class CompareTool : public ToolWidget
        {
            Q_OBJECT

        public:
            CompareTool(
                const QMap<QString, QAction*>&,
                App*,
                QWidget* parent = nullptr);

            ~CompareTool() override;

        public Q_SLOTS:
            void setCompareOptions(const tl::timeline::CompareOptions&);

        private Q_SLOTS:
            void _activatedCallback(const QModelIndex&);

        Q_SIGNALS:
            void compareOptionsChanged(const tl::timeline::CompareOptions&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
