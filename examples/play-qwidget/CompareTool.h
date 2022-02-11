// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include "FilesModel.h"
#include "ToolWidget.h"

#include <tlrQWidget/RadioButtonGroup.h>

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
        void _modeCallback(const QVariant&);
        void _wipeXSpinBoxCallback(double);
        void _wipeXSliderCallback(int);
        void _wipeYSpinBoxCallback(double);
        void _wipeYSliderCallback(int);
        void _wipeRotationSpinBoxCallback(double);
        void _wipeRotationSliderCallback(int);

    Q_SIGNALS:
        void compareOptionsChanged(const tlr::render::CompareOptions&);

    private:
        void _widgetUpdate();

        std::shared_ptr<FilesModel> _filesModel;
        FilesBModel* _filesBModel = nullptr;
        render::CompareOptions _compareOptions;
        QTreeView* _treeView = nullptr;
        qwidget::RadioButtonGroup* _modeButtonGroup = nullptr;
        QDoubleSpinBox* _wipeXSpinBox = nullptr;
        QSlider* _wipeXSlider = nullptr;
        QDoubleSpinBox* _wipeYSpinBox = nullptr;
        QSlider* _wipeYSlider = nullptr;
        QDoubleSpinBox* _wipeRotationSpinBox = nullptr;
        QSlider* _wipeRotationSlider = nullptr;
    };
}
