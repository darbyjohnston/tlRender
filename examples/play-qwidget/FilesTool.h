// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"

#include <QListView>
#include <QToolButton>

namespace tlr
{
    //! Files tool.
    class FilesTool : public QWidget
    {
        Q_OBJECT

    public:
        FilesTool(
            FilesModel*,
            QWidget* parent = nullptr);

    private Q_SLOTS:
        void _activatedCallback(const QModelIndex&);
        void _countCallback();

    private:
        void _countUpdate();

        FilesModel* _filesModel = nullptr;
        QListView* _listView = nullptr;
        QToolButton* _closeButton = nullptr;
        QToolButton* _closeAllButton = nullptr;
        QToolButton* _nextButton = nullptr;
        QToolButton* _prevButton = nullptr;
    };
}
