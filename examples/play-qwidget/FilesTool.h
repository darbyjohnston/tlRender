// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"

#include <QComboBox>
#include <QSlider>
#include <QTreeView>

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

    public Q_SLOTS:
        void setCompareOptions(const tlr::render::CompareOptions&);

    private Q_SLOTS:
        void _activatedCallback(const QModelIndex&);
        void _compareCallback(int);
        void _sliderCallback(int);
        void _countCallback();

    Q_SIGNALS:
        void compareOptionsChanged(const tlr::render::CompareOptions&);

    private:
        void _widgetUpdate();

        FilesModel* _filesModel = nullptr;
        render::CompareOptions _compareOptions;
        QTreeView* _treeView = nullptr;
        QComboBox* _compareComboBox = nullptr;
        QSlider* _compareSlider = nullptr;
    };
}
