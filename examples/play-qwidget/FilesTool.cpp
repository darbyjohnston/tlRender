// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "App.h"
#include "FilesTool.h"
#include "FilesView.h"

#include <QBoxLayout>
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
        _compareSpinBox = new QDoubleSpinBox;
        _compareSpinBox->setRange(0.0, 1.0);
        _compareSpinBox->setSingleStep(0.1);
        _compareSlider = new QSlider(Qt::Orientation::Horizontal);
        _compareSlider->setRange(0, 100);

        auto layout = new QVBoxLayout;
        layout->addWidget(_treeView);
        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(_compareComboBox);
        auto hLayout2 = new QHBoxLayout;
        hLayout2->addWidget(_compareSpinBox);
        hLayout2->addWidget(_compareSlider, 1);
        hLayout->addLayout(hLayout2, 1);
        layout->addLayout(hLayout);
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
            _compareSpinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_compareSpinBoxCallback(double)));
        connect(
            _compareSlider,
            SIGNAL(valueChanged(int)),
            SLOT(_compareSliderCallback(int)));

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

    void FilesTool::_compareSpinBoxCallback(double value)
    {
        _compareOptions.wipe = value;
        _widgetUpdate();
        Q_EMIT compareOptionsChanged(_compareOptions);
    }

    void FilesTool::_compareSliderCallback(int value)
    {
        _compareOptions.wipe = value / 100.F;
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
        _compareComboBox->setEnabled(count > 0);
        {
            QSignalBlocker signalBlocker(_compareComboBox);
            _compareComboBox->setCurrentIndex(static_cast<int>(_compareOptions.mode));
        }
        _compareSpinBox->setEnabled(count > 0 && render::CompareMode::Wipe == _compareOptions.mode);
        {
            QSignalBlocker signalBlocker(_compareSpinBox);
            _compareSpinBox->setValue(_compareOptions.wipe);
        }
        _compareSlider->setEnabled(count > 0 && render::CompareMode::Wipe == _compareOptions.mode);
        {
            QSignalBlocker signalBlocker(_compareSlider);
            _compareSlider->setValue(_compareOptions.wipe * 100);
        }
    }
}
