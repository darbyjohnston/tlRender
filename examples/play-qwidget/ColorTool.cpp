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

        _inputComboBox = new QComboBox;

        _displayComboBox = new QComboBox;

        _viewComboBox = new QComboBox;

        auto layout = new QVBoxLayout;
        layout->addWidget(new QLabel(tr("Configuration")));
        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(_fileNameLineEdit);
        hLayout->addWidget(fileNameButton);
        layout->addLayout(hLayout);
        layout->addWidget(new QLabel(tr("Input")));
        layout->addWidget(_inputComboBox);
        layout->addWidget(new QLabel(tr("Display")));
        layout->addWidget(_displayComboBox);
        layout->addWidget(new QLabel(tr("View")));
        layout->addWidget(_viewComboBox);
        auto widget = new QWidget;
        widget->setLayout(layout);
        addWidget(widget);
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
            _inputComboBox,
            QOverload<int>::of(&QComboBox::activated),
            [this](int index)
            {
                _colorModel->setInputIndex(index);
            });

        connect(
            _displayComboBox,
            QOverload<int>::of(&QComboBox::activated),
            [this](int index)
            {
                _colorModel->setDisplayIndex(index);
            });

        connect(
            _viewComboBox,
            QOverload<int>::of(&QComboBox::activated),
            [this](int index)
            {
                _colorModel->setViewIndex(index);
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
        {
            QSignalBlocker blocker(_inputComboBox);
            _inputComboBox->clear();
            for (const auto& i : _data.inputs)
            {
                _inputComboBox->addItem(QString::fromUtf8(i.c_str()));
            }
            _inputComboBox->setCurrentIndex(_data.inputIndex);
        }
        {
            QSignalBlocker blocker(_displayComboBox);
            _displayComboBox->clear();
            for (const auto& i : _data.displays)
            {
                _displayComboBox->addItem(QString::fromUtf8(i.c_str()));
            }
            _displayComboBox->setCurrentIndex(_data.displayIndex);
        }
        {
            QSignalBlocker blocker(_viewComboBox);
            _viewComboBox->clear();
            for (const auto& i : _data.views)
            {
                _viewComboBox->addItem(QString::fromUtf8(i.c_str()));
            }
            _viewComboBox->setCurrentIndex(_data.viewIndex);
        }
    }
}
