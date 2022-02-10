// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"
#include "CompareTool.h"
#include "FilesView.h"

#include <QBoxLayout>
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

        _compareComboBox = new QComboBox;
        for (const auto& i : render::getCompareModeLabels())
        {
            _compareComboBox->addItem(QString::fromUtf8(i.c_str()));
        }
        _horizontalSlider = new QSlider(Qt::Orientation::Horizontal);
        _horizontalSlider->setRange(0, sliderSteps);
        _verticalSlider = new QSlider(Qt::Orientation::Horizontal);
        _verticalSlider->setRange(0, sliderSteps);
        _freeXSlider = new QSlider(Qt::Orientation::Horizontal);
        _freeXSlider->setRange(0, sliderSteps);
        _freeYSlider = new QSlider(Qt::Orientation::Horizontal);
        _freeYSlider->setRange(0, sliderSteps);
        _freeRotSpinBox = new QDoubleSpinBox;
        _freeRotSpinBox->setRange(0.0, 360.0);
        _freeRotSpinBox->setSingleStep(10.0);
        _freeRotSpinBox->setToolTip(tr("Rotation"));

        auto layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(_treeView);
        auto vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(10, 10, 10, 10);
        vLayout->setSpacing(10);
        vLayout->addWidget(_compareComboBox);
        vLayout->addWidget(new QLabel(tr("Horizontal")));
        vLayout->addWidget(_horizontalSlider);
        vLayout->addWidget(new QLabel(tr("Vertical")));
        vLayout->addWidget(_verticalSlider);
        vLayout->addWidget(new QLabel(tr("Free")));
        vLayout->addWidget(_freeXSlider);
        vLayout->addWidget(_freeYSlider);
        vLayout->addWidget(_freeRotSpinBox);
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
            _compareComboBox,
            SIGNAL(activated(int)),
            SLOT(_compareCallback(int)));
        connect(
            _horizontalSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_horizontalSliderCallback(int)));
        connect(
            _verticalSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_verticalSliderCallback(int)));
        connect(
            _freeXSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_freeXSliderCallback(int)));
        connect(
            _freeYSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_freeYSliderCallback(int)));
        connect(
            _freeRotSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_freeRotSpinBoxCallback(double)));
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

    void CompareTool::_compareCallback(int value)
    {
        _compareOptions.mode = static_cast<render::CompareMode>(value);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_horizontalSliderCallback(int value)
    {
        _compareOptions.horizontal = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_verticalSliderCallback(int value)
    {
        _compareOptions.vertical = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_freeXSliderCallback(int value)
    {
        _compareOptions.free.x = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_freeYSliderCallback(int value)
    {
        _compareOptions.free.y = value / static_cast<float>(sliderSteps);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_freeRotSpinBoxCallback(double value)
    {
        _compareOptions.freeRot = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void CompareTool::_widgetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_compareComboBox);
            _compareComboBox->setCurrentIndex(static_cast<int>(_compareOptions.mode));
        }
        {
            QSignalBlocker signalBlocker(_horizontalSlider);
            _horizontalSlider->setValue(_compareOptions.horizontal * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_verticalSlider);
            _verticalSlider->setValue(_compareOptions.vertical * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_freeXSlider);
            _freeXSlider->setValue(_compareOptions.free.x * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_freeYSlider);
            _freeYSlider->setValue(_compareOptions.free.y * sliderSteps);
        }
        {
            QSignalBlocker signalBlocker(_freeRotSpinBox);
            _freeRotSpinBox->setValue(_compareOptions.freeRot);
        }
    }
}
