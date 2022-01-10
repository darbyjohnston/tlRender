// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "AudioSyncWidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>

namespace tlr
{
    AudioOffsetWidget::AudioOffsetWidget(QWidget* parent) :
        QWidget(parent)
    {
        _slider = new QSlider;
        _slider->setOrientation(Qt::Horizontal);
        _slider->setRange(-100, 100);

        _spinBox = new QDoubleSpinBox;
        _spinBox->setRange(-1.0, 1.0);
        _spinBox->setSingleStep(0.1);

        auto resetButton = new QPushButton(tr("Reset"));

        auto layout = new QVBoxLayout;
        layout->addWidget(_slider);
        auto hLayout = new QHBoxLayout;
        hLayout->addWidget(_spinBox);
        hLayout->addWidget(resetButton);
        layout->addLayout(hLayout);
        layout->addStretch();
        setLayout(layout);

        connect(
            _slider,
            SIGNAL(valueChanged(int)),
            SLOT(_sliderCallback(int)));

        connect(
            _spinBox,
            SIGNAL(valueChanged(double)),
            SLOT(_spinBoxCallback(double)));

        connect(
            resetButton,
            SIGNAL(clicked()),
            SLOT(_resetCallback()));
    }

    void AudioOffsetWidget::_sliderCallback(int value)
    {
        _offset = value / 100.0;
        Q_EMIT offsetChanged(_offset);
        _offsetUpdate();
    }

    void AudioOffsetWidget::_spinBoxCallback(double value)
    {
        _offset = value;
        Q_EMIT offsetChanged(_offset);
        _offsetUpdate();
    }

    void AudioOffsetWidget::_resetCallback()
    {
        _offset = 0.0;
        Q_EMIT offsetChanged(_offset);
        _offsetUpdate();
    }

    void AudioOffsetWidget::_offsetUpdate()
    {
        {
            QSignalBlocker signalBlocker(_spinBox);
            _spinBox->setValue(_offset);
        }
        {
            QSignalBlocker signalBlocker(_slider);
            _slider->setValue(_offset * 100);
        }
    }

    AudioSyncWidget::AudioSyncWidget(QWidget* parent) :
        QToolBox(parent)
    {
        auto offsetWidget = new AudioOffsetWidget;
        addItem(offsetWidget, tr("Offset"));

        connect(
            offsetWidget,
            SIGNAL(offsetChanged(double)),
            SIGNAL(audioOffsetChanged(double)));

        connect(
            this,
            SIGNAL(currentChanged(int)),
            SLOT(_currentItemCallback(int)));

        QSettings settings;
        setCurrentIndex(settings.value("AudioSync/CurrentItem").toInt());
    }

    void AudioSyncWidget::_currentItemCallback(int value)
    {
        QSettings settings;
        settings.setValue("AudioSync/CurrentItem", value);
    }
}
