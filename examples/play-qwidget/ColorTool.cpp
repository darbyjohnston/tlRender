// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "ColorTool.h"

#include <tlrCore/Path.h>

#include <QBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QToolButton>

namespace tlr
{
    ColorTool::ColorTool(
        const std::shared_ptr<ColorModel>& colorModel,
        QWidget* parent) :
        ToolWidget(parent),
        _colorModel(colorModel)
    {
        _fileNameLineEdit = new QLineEdit;
        auto fileNameButton = new QToolButton;
        fileNameButton->setIcon(QIcon(":/Icons/FileBrowser.svg"));
        fileNameButton->setAutoRaise(true);

        _inputListView = new QListView;
        _inputListView->setAlternatingRowColors(true);
        _inputListView->setSelectionMode(QAbstractItemView::NoSelection);
        _inputListView->setModel(new ColorInputListModel(colorModel, this));

        _displayListView = new QListView;
        _displayListView->setAlternatingRowColors(true);
        _displayListView->setSelectionMode(QAbstractItemView::NoSelection);
        _displayListView->setModel(new ColorDisplayListModel(colorModel, this));

        _viewListView = new QListView;
        _viewListView->setAlternatingRowColors(true);
        _viewListView->setSelectionMode(QAbstractItemView::NoSelection);
        _viewListView->setModel(new ColorViewListModel(colorModel, this));

        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(_fileNameLineEdit);
        hLayout->addWidget(fileNameButton);
        auto widget = new QWidget;
        widget->setLayout(hLayout);
        addBellows(tr("Configuration"), widget);
        addBellows(tr("Input"), _inputListView);
        addBellows(tr("Display"), _displayListView);
        addBellows(tr("View"), _viewListView);
        addStretch();

        connect(
            fileNameButton,
            &QToolButton::clicked,
            [this]
            {
                QString dir;
                if (!_data.fileName.empty())
                {
                    dir = QString::fromUtf8(file::Path(_data.fileName).get().c_str());
                }

                const auto fileName = QFileDialog::getOpenFileName(
                    window(),
                    tr("Open"),
                    dir,
                    tr("Files") + " (*.ocio)");
                if (!fileName.isEmpty())
                {
                    _colorModel->setConfig(fileName.toUtf8().data());
                }
            });

        connect(
            _fileNameLineEdit,
            &QLineEdit::editingFinished,
            [this]
            {
                _colorModel->setConfig(_fileNameLineEdit->text().toUtf8().data());
            });

        connect(
            _inputListView,
            &QAbstractItemView::activated,
            [this](const QModelIndex& index)
            {
                _colorModel->setInputIndex(index.row());
            });

        connect(
            _displayListView,
            &QAbstractItemView::activated,
            [this](const QModelIndex& index)
            {
                _colorModel->setDisplayIndex(index.row());
            });

        connect(
            _viewListView,
            &QAbstractItemView::activated,
            [this](const QModelIndex& index)
            {
                _colorModel->setViewIndex(index.row());
            });

        _dataObserver = observer::ValueObserver<ColorModelData>::create(
            colorModel->observeData(),
            [this](const ColorModelData& value)
            {
                _data = value;
                _widgetUpdate();
            });
    }

    ColorTool::~ColorTool()
    {}

    void ColorTool::_widgetUpdate()
    {
        {
            QSignalBlocker blocker(_fileNameLineEdit);
            _fileNameLineEdit->setText(QString::fromUtf8(_data.fileName.c_str()));
        }
    }
}
