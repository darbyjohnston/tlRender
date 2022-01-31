// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"
#include "ToolWidget.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QTreeView>

namespace tlr
{
    //! Files tool.
    class FilesTool : public ToolWidget
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
        void _horizontalSliderCallback(int);
        void _verticalSliderCallback(int);
        void _freePosXSpinBoxCallback(double);
        void _freePosYSpinBoxCallback(double);
        void _freeRotSpinBoxCallback(double);
        void _countCallback();

    Q_SIGNALS:
        void compareOptionsChanged(const tlr::render::CompareOptions&);

    private:
        void _widgetUpdate();

        FilesModel* _filesModel = nullptr;
        render::CompareOptions _compareOptions;
        QTreeView* _treeView = nullptr;
        QComboBox* _compareComboBox = nullptr;
        QSlider* _horizontalSlider = nullptr;
        QSlider* _verticalSlider = nullptr;
        QDoubleSpinBox* _freePosXSpinBox = nullptr;
        QDoubleSpinBox* _freePosYSpinBox = nullptr;
        QDoubleSpinBox* _freeRotSpinBox = nullptr;
    };
}
