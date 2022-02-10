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

        _modeButtonGroup = new QButtonGroup;
        _modeButtonGroup->setExclusive(true);
        for (const auto i : render::getCompareModeEnums())
        {
            auto button = new QRadioButton;
            button->setText(QString::fromUtf8(
                render::getCompareModeLabels()[static_cast<size_t>(i)].c_str()));
            _modeToButton[i] = button;
            _buttonToMode[button] = i;
            _modeButtonGroup->addButton(button);
        }

        _wipeXSlider = new QSlider(Qt::Orientation::Horizontal);
        _wipeXSlider->setRange(0, sliderSteps);
        _wipeYSlider = new QSlider(Qt::Orientation::Horizontal);
        _wipeYSlider->setRange(0, sliderSteps);
        _wipeRotationSpinBox = new QDoubleSpinBox;
        _wipeRotationSpinBox->setRange(0.0, 360.0);
        _wipeRotationSpinBox->setSingleStep(10.0);
        _wipeRotationSpinBox->setToolTip(tr("Rotation"));

        auto layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(_treeView);
        auto vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(10, 10, 10, 10);
        vLayout->setSpacing(10);
        auto hLayout = new QHBoxLayout;
        for (const auto i : render::getCompareModeEnums())
        {
            hLayout->addWidget(_modeToButton[i]);
        }
        hLayout->addStretch();
        vLayout->addLayout(hLayout);
        vLayout->addWidget(new QLabel(tr("Wipe")));
        auto formLayout = new QFormLayout;
        formLayout->addRow(tr("X:"), _wipeXSlider);
        formLayout->addRow(tr("Y:"), _wipeYSlider);
        formLayout->addRow(tr("Rotation:"), _wipeRotationSpinBox);
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
            SIGNAL(buttonToggled(QAbstractButton*, bool)),
            SLOT(_modeCallback(QAbstractButton*, bool)));

        connect(
            _wipeXSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_wipeXCallback(int)));
        connect(
            _wipeYSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_wipeYCallback(int)));
        connect(
            _wipeRotationSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_wipeRotationCallback(double)));
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

    void CompareTool::_modeCallback(QAbstractButton* button, bool value)
    {
        const auto i = _buttonToMode.find(button);
        if (i != _buttonToMode.end())
        {
            _compareOptions.mode = i.value();
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(_compareOptions);
        }
    }

    void CompareTool::_wipeXCallback(int value)
    {
        _compareOptions.wipeCenter.x = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeYCallback(int value)
    {
        _compareOptions.wipeCenter.y = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_wipeRotationCallback(double value)
    {
        _compareOptions.wipeRotation = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_modeButtonGroup);
            const auto i = _modeToButton.find(_compareOptions.mode);
            if (i != _modeToButton.end())
            {
                i.value()->setChecked(true);
            }
        }
        {
            QSignalBlocker signalBlocker(_wipeXSlider);
            _wipeXSlider->setValue(_compareOptions.wipeCenter.x * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_wipeYSlider);
            _wipeYSlider->setValue(_compareOptions.wipeCenter.y * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_wipeRotationSpinBox);
            _wipeRotationSpinBox->setValue(_compareOptions.wipeRotation);
        }
    }
}
