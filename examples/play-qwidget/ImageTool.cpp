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
        _yuvRangeComboBox = new QComboBox;
        _yuvRangeComboBox->addItem(tr("From File"));
        _yuvRangeComboBox->addItem(tr("Full"));
        _yuvRangeComboBox->addItem(tr("Video"));

        auto layout = new QVBoxLayout;
        layout->addWidget(_yuvRangeComboBox);
        layout->addStretch();
        setLayout(layout);

        _yuvRangeComboBox->setCurrentIndex(static_cast<int>(render::YUVRange::FromFile));

        connect(
            _yuvRangeComboBox,
            SIGNAL(activated(int)),
            SLOT(_yuvRangeCallback(int)));
    }

    void YUVRangeWidget::_yuvRangeCallback(int value)
    {
        Q_EMIT yuvRangeChanged(static_cast<render::YUVRange>(value));
    }

    ImageTool::ImageTool(QWidget* parent) :
        QToolBox(parent)
    {
        auto yuvRangeWidget = new YUVRangeWidget;
        addItem(yuvRangeWidget, tr("YUV Range"));

        connect(
            yuvRangeWidget,
            SIGNAL(yuvRangeChanged(tlr::render::YUVRange)),
            SLOT(_yuvRangeCallback(tlr::render::YUVRange)));

        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("ImageTool/CurrentItem").toInt());
    }

    void ImageTool::_yuvRangeCallback(render::YUVRange value)
    {
        _imageOptions.yuvRange = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageTool::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("ImageTool/CurrentItem", value);
    }
}
