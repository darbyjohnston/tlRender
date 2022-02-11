// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"
#include "CompareTool.h"
#include "FilesView.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QSettings>

namespace tlr
{
    namespace
    {
        const size_t sliderSteps = 1000;
    }

    CompareTool::CompareTool(
        const std::shared_ptr<FilesModel>& filesModel,
        const std::shared_ptr<core::Context>& context,
        QWidget* parent) :
        ToolWidget(parent),
        _filesModel(filesModel)
    {
        _filesBModel = new FilesBModel(filesModel, context, this);

        _treeView = new QTreeView;
        _treeView->setAllColumnsShowFocus(true);
        _treeView->setAlternatingRowColors(true);
        _treeView->setSelectionMode(QAbstractItemView::NoSelection);
        _treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
        _treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
        _treeView->setIndentation(0);
        //! \bug Setting the model causes this output to be printed on exit:
        //! QBasicTimer::start: QBasicTimer can only be used with threads started with QThread
        _treeView->setModel(_filesBModel);

        _modeButtonGroup = new qwidget::RadioButtonGroup;
        for (const auto i : render::getCompareModeEnums())
        {
            _modeButtonGroup->addButton(
                QString::fromUtf8(render::getLabel(i).c_str()),
                QVariant::fromValue<render::CompareMode>(i));
        }

        _wipeXSpinBox = new QDoubleSpinBox;
        _wipeXSpinBox->setRange(0.0, 1.0);
        _wipeXSpinBox->setSingleStep(0.1);
        _wipeXSlider = new QSlider(Qt::Orientation::Horizontal);
        _wipeXSlider->setRange(0, sliderSteps);

        _wipeYSpinBox = new QDoubleSpinBox;
        _wipeYSpinBox->setRange(0.0, 1.0);
        _wipeYSpinBox->setSingleStep(0.1);
        _wipeYSlider = new QSlider(Qt::Orientation::Horizontal);
        _wipeYSlider->setRange(0, sliderSteps);

        _wipeRotationSpinBox = new QDoubleSpinBox;
        _wipeRotationSpinBox->setRange(0.0, 360.0);
        _wipeRotationSpinBox->setSingleStep(10.0);
        _wipeRotationSlider = new QSlider(Qt::Orientation::Horizontal);
        _wipeRotationSlider->setRange(0, sliderSteps);

        auto layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(_treeView);
        auto vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(10, 10, 10, 10);
        vLayout->setSpacing(10);
        vLayout->addWidget(_modeButtonGroup);
        vLayout->addWidget(new QLabel(tr("Wipe")));
        auto formLayout = new QFormLayout;
        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(_wipeXSpinBox);
        hLayout->addWidget(_wipeXSlider);
        formLayout->addRow(tr("X:"), hLayout);
        hLayout = new QHBoxLayout;
        hLayout->addWidget(_wipeYSpinBox);
        hLayout->addWidget(_wipeYSlider);
        formLayout->addRow(tr("Y:"), hLayout);
        hLayout = new QHBoxLayout;
        hLayout->addWidget(_wipeRotationSpinBox);
        hLayout->addWidget(_wipeRotationSlider);
        formLayout->addRow(tr("Rotation:"), hLayout);
        vLayout->addLayout(formLayout);
        layout->addLayout(vLayout);
        auto widget = new QWidget;
        widget->setLayout(layout);
        addWidget(widget, 1);

        _widgetUpdate();

        QSettings settings;
        auto ba = settings.value(qt::versionedSettingsKey("CompareTool/Header")).toByteArray();
        if (!ba.isEmpty())
        {
            _treeView->header()->restoreState(ba);
        }

        connect(
            _treeView,
            SIGNAL(activated(const QModelIndex&)),
            SLOT(_activatedCallback(const QModelIndex&)));

        connect(
            _modeButtonGroup,
            SIGNAL(checked(const QVariant&)),
            SLOT(_modeCallback(const QVariant&)));

        connect(
            _wipeXSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_wipeXSpinBoxCallback(double)));
        connect(
            _wipeXSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_wipeXSliderCallback(int)));

        connect(
            _wipeYSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_wipeYSpinBoxCallback(double)));
        connect(
            _wipeYSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_wipeYSliderCallback(int)));

        connect(
            _wipeRotationSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_wipeRotationSpinBoxCallback(double)));
        connect(
            _wipeRotationSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_wipeRotationSliderCallback(int)));
    }

    CompareTool::~CompareTool()
    {
        QSettings settings;
        settings.setValue(qt::versionedSettingsKey("CompareTool/Header"), _treeView->header()->saveState());
    }

    void CompareTool::setCompareOptions(const render::CompareOptions& value)
    {
        if (value == _compareOptions)
            return;
        _compareOptions = value;
        _widgetUpdate();
    }

    void CompareTool::_activatedCallback(const QModelIndex& index)
    {
        _filesModel->toggleB(index.row());
    }

    void CompareTool::_modeCallback(const QVariant& value)
    {
        _compareOptions.mode = value.value<render::CompareMode>();
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeXSpinBoxCallback(double value)
    {
        _compareOptions.wipeCenter.x = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeXSliderCallback(int value)
    {
        _compareOptions.wipeCenter.x = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeYSliderCallback(int value)
    {
        _compareOptions.wipeCenter.y = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeYSpinBoxCallback(double value)
    {
        _compareOptions.wipeCenter.y = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeRotationSpinBoxCallback(double value)
    {
        _compareOptions.wipeRotation = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeRotationSliderCallback(int value)
    {
        _compareOptions.wipeRotation = value / static_cast<float>(sliderSteps) * 360.F;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_modeButtonGroup);
            _modeButtonGroup->setChecked(QVariant::fromValue<render::CompareMode>(_compareOptions.mode));
        }
        {
            QSignalBlocker signalBlocker(_wipeXSpinBox);
            _wipeXSpinBox->setValue(_compareOptions.wipeCenter.x);
        }
        {
            QSignalBlocker signalBlocker(_wipeXSlider);
            _wipeXSlider->setValue(_compareOptions.wipeCenter.x * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_wipeYSpinBox);
            _wipeYSpinBox->setValue(_compareOptions.wipeCenter.y);
        }
        {
            QSignalBlocker signalBlocker(_wipeYSlider);
            _wipeYSlider->setValue(_compareOptions.wipeCenter.y * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_wipeRotationSpinBox);
            _wipeRotationSpinBox->setValue(_compareOptions.wipeRotation);
        }
        {
            QSignalBlocker signalBlocker(_wipeYSlider);
            _wipeRotationSlider->setValue(_compareOptions.wipeRotation / 360.F * sliderSteps);
        }
    }
}
