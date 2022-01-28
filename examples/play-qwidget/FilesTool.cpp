// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"
#include "FilesTool.h"
#include "FilesView.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSignalBlocker>

namespace tlr
{
    FilesTool::FilesTool(
        FilesModel* filesModel,
        QWidget* parent) :
        QWidget(parent),
        _filesModel(filesModel)
    {
        _treeView = new QTreeView;
        _treeView->setAllColumnsShowFocus(true);
        _treeView->setAlternatingRowColors(true);
        _treeView->setSelectionMode(QAbstractItemView::NoSelection);
        _treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
        _treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
        _treeView->setIndentation(0);
        _treeView->setModel(filesModel);
        _treeView->resizeColumnToContents(2);
        _treeView->resizeColumnToContents(3);

        _compareComboBox = new QComboBox;
        for (const auto& i : render::getCompareModeLabels())
        {
            _compareComboBox->addItem(QString::fromUtf8(i.c_str()));
        }
        _horizontalSlider = new QSlider(Qt::Orientation::Horizontal);
        _horizontalSlider->setRange(0, 1000);
        _verticalSlider = new QSlider(Qt::Orientation::Horizontal);
        _verticalSlider->setRange(0, 1000);
        _freePosXSpinBox = new QDoubleSpinBox;
        _freePosXSpinBox->setRange(0.0, 2000.0);
        _freePosXSpinBox->setSingleStep(10.0);
        _freePosXSpinBox->setToolTip(tr("X position"));
        _freePosYSpinBox = new QDoubleSpinBox;
        _freePosYSpinBox->setRange(0.0, 2000.0);
        _freePosYSpinBox->setSingleStep(10.0);
        _freePosYSpinBox->setToolTip(tr("Y position"));
        _freeRotSpinBox = new QDoubleSpinBox;
        _freeRotSpinBox->setRange(0.0, 360.0);
        _freeRotSpinBox->setSingleStep(10.0);
        _freeRotSpinBox->setToolTip(tr("Rotation"));

        auto layout = new QVBoxLayout;
        layout->addWidget(_treeView);
        auto formLayout = new QFormLayout;
        formLayout->addRow(tr("Mode:"), _compareComboBox);
        formLayout->addRow(tr("Horizontal:"), _horizontalSlider);
        formLayout->addRow(tr("Vertical:"), _verticalSlider);
        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(_freePosXSpinBox);
        hLayout->addWidget(_freePosYSpinBox);
        hLayout->addWidget(_freeRotSpinBox);
        formLayout->addRow(tr("Free:"), hLayout);
        auto groupBox = new QGroupBox(tr("Compare"));
        groupBox->setLayout(formLayout);
        layout->addWidget(groupBox);
        setLayout(layout);

        _widgetUpdate();

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
            _freePosXSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_freePosXSpinBoxCallback(double)));
        connect(
            _freePosYSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_freePosYSpinBoxCallback(double)));
        connect(
            _freeRotSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_freeRotSpinBoxCallback(double)));

        connect(
            filesModel,
            SIGNAL(countChanged(int)),
            SLOT(_countCallback()));
    }

    void FilesTool::setCompareOptions(const render::CompareOptions& value)
    {
        if (value == _compareOptions)
            return;
        _compareOptions = value;
        _widgetUpdate();
    }

    void FilesTool::_activatedCallback(const QModelIndex& index)
    {
        _filesModel->setA(index.row());
    }

    void FilesTool::_compareCallback(int value)
    {
        _compareOptions.mode = static_cast<render::CompareMode>(value);
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void FilesTool::_horizontalSliderCallback(int value)
    {
        _compareOptions.horizontal = value / 1000.0;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void FilesTool::_verticalSliderCallback(int value)
    {
        _compareOptions.vertical = value / 1000.0;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void FilesTool::_freePosXSpinBoxCallback(double value)
    {
        _compareOptions.freePos.x = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void FilesTool::_freePosYSpinBoxCallback(double value)
    {
        _compareOptions.freePos.y = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void FilesTool::_freeRotSpinBoxCallback(double value)
    {
        _compareOptions.freeRot = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void FilesTool::_countCallback()
    {
        _widgetUpdate();
    }

    void FilesTool::_widgetUpdate()
    {
        const int count = _filesModel->rowCount();
        {
            QSignalBlocker signalBlocker(_compareComboBox);
            _compareComboBox->setCurrentIndex(static_cast<int>(_compareOptions.mode));
        }
        {
            QSignalBlocker signalBlocker(_horizontalSlider);
            _horizontalSlider->setValue(_compareOptions.horizontal * 1000.F);
        }
        {
            QSignalBlocker signalBlocker(_verticalSlider);
            _verticalSlider->setValue(_compareOptions.vertical * 1000.F);
        }
        {
            QSignalBlocker signalBlocker(_freePosXSpinBox);
            _freePosXSpinBox->setValue(_compareOptions.freePos.x);
        }
        {
            QSignalBlocker signalBlocker(_freePosYSpinBox);
            _freePosYSpinBox->setValue(_compareOptions.freePos.y);
        }
        {
            QSignalBlocker signalBlocker(_freeRotSpinBox);
            _freeRotSpinBox->setValue(_compareOptions.freeRot);
        }
    }
}
