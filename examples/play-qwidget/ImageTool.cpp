// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "ImageTool.h"

#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>

namespace tlr
{
    YUVRangeWidget::YUVRangeWidget(QWidget* parent) :
        QWidget(parent)
    {
        _comboBox = new QComboBox;
        for (const auto& i : render::getYUVRangeLabels())
        {
            _comboBox->addItem(QString::fromUtf8(i.c_str()));
        }

        auto layout = new QVBoxLayout;
        layout->addWidget(_comboBox);
        layout->addStretch();
        setLayout(layout);

        _valueUpdate();

        connect(
            _comboBox,
            SIGNAL(activated(int)),
            SLOT(_callback(int)));
    }

    void YUVRangeWidget::setValue(render::YUVRange value)
    {
        if (value == _value)
            return;
        _value = value;
        _valueUpdate();
    }

    void YUVRangeWidget::_callback(int value)
    {
        _value = static_cast<render::YUVRange>(value);
        Q_EMIT valueChanged(_value);
    }

    void YUVRangeWidget::_valueUpdate()
    {
        _comboBox->setCurrentIndex(static_cast<int>(_value));
    }

    ImageTool::ImageTool(QWidget* parent) :
        QToolBox(parent)
    {
        _yuvRangeWidget = new YUVRangeWidget;
        addItem(_yuvRangeWidget, tr("YUV Range"));

        _optionsUpdate();

        connect(
            _yuvRangeWidget,
            SIGNAL(valueChanged(tlr::render::YUVRange)),
            SLOT(_yuvRangeCallback(tlr::render::YUVRange)));

        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("ImageTool/CurrentItem").toInt());
    }

    void ImageTool::setOptions(const render::ImageOptions& options)
    {
        if (options == _options)
            return;
        _options = options;
        _optionsUpdate();
    }

    void ImageTool::_yuvRangeCallback(render::YUVRange value)
    {
        _options.yuvRange = value;
        Q_EMIT optionsChanged(_options);
    }

    void ImageTool::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("ImageTool/CurrentItem", value);
    }

    void ImageTool::_optionsUpdate()
    {
        _yuvRangeWidget->setValue(_options.yuvRange);
    }
}
