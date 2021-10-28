// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "ImageOptionsWidget.h"


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

        _yuvRangeComboBox->setCurrentIndex(static_cast<int>(gl::YUVRange::FromFile));

        connect(
            _yuvRangeComboBox,
            SIGNAL(activated(int)),
            SLOT(_yuvRangeCallback(int)));
    }

    void YUVRangeWidget::_yuvRangeCallback(int value)
    {
        Q_EMIT(yuvRangeChanged(static_cast<gl::YUVRange>(value)));
    }

    ImageOptionsWidget::ImageOptionsWidget(QWidget* parent) :
        QToolBox(parent)
    {
        auto yuvRangeWidget = new YUVRangeWidget;
        addItem(yuvRangeWidget, tr("YUV Range"));

        connect(
            yuvRangeWidget,
            SIGNAL(yuvRangeChanged(tlr::gl::YUVRange)),
            SLOT(_yuvRangeCallback(tlr::gl::YUVRange)));
        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("ImageOptions/CurrentItem").toInt());
    }

    void ImageOptionsWidget::_yuvRangeCallback(tlr::gl::YUVRange value)
    {
        _imageOptions.yuvRange = value;
        Q_EMIT imageOptionsChanged(_imageOptions);
    }

    void ImageOptionsWidget::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("ImageOptions/CurrentItem", value);
    }
}
