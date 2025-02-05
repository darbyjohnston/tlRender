// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/Util.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Search widget.
        class SearchWidget : public QWidget
        {
            Q_OBJECT

        public:
            SearchWidget(QWidget* parent = nullptr);

            virtual ~SearchWidget();

        public Q_SLOTS:
            //! Clear the search.
            void clear();

        Q_SIGNALS:
            //! This signal is emitted when the search is changed.
            void searchChanged(const QString&);

        private:
            void _widgetUpdate();

            DTK_PRIVATE();
        };
    }
}
