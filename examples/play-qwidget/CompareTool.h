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
    //! Compare tool.
    class CompareTool : public ToolWidget
    {
        Q_OBJECT

    public:
        CompareTool(
            const std::shared_ptr<FilesModel>&,
            const std::shared_ptr<core::Context>&,
            QWidget* parent = nullptr);

        ~CompareTool() override;

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

    Q_SIGNALS:
        void compareOptionsChanged(const tlr::render::CompareOptions&);

    private:
        void _widgetUpdate();

        std::shared_ptr<FilesModel> _filesModel;
        FilesBModel* _filesBModel = nullptr;
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
