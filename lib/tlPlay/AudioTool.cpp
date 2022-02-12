// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/AudioTool.h>

#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>

namespace tl
{
    namespace play
    {
        struct AudioOffsetWidget::Private
        {
            double offset = 0.0;
            QDoubleSpinBox* spinBox = nullptr;
            QSlider* slider = nullptr;
        };

        AudioOffsetWidget::AudioOffsetWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.slider = new QSlider;
            p.slider->setOrientation(Qt::Horizontal);
            p.slider->setRange(-100, 100);

            p.spinBox = new QDoubleSpinBox;
            p.spinBox->setRange(-1.0, 1.0);
            p.spinBox->setSingleStep(0.1);

            auto resetButton = new QPushButton(tr("Reset"));

            auto layout = new QVBoxLayout;
            layout->addWidget(p.slider);
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.spinBox, 1);
            hLayout->addWidget(resetButton);
            layout->addLayout(hLayout);
            layout->addStretch();
            setLayout(layout);

            connect(
                p.slider,
                SIGNAL(valueChanged(int)),
                SLOT(_sliderCallback(int)));

            connect(
                p.spinBox,
                SIGNAL(valueChanged(double)),
                SLOT(_spinBoxCallback(double)));

            connect(
                resetButton,
                SIGNAL(clicked()),
                SLOT(_resetCallback()));
        }

        void AudioOffsetWidget::setAudioOffset(double value)
        {
            TLRENDER_P();
            p.offset = value;
            _offsetUpdate();
        }

        void AudioOffsetWidget::_sliderCallback(int value)
        {
            TLRENDER_P();
            p.offset = value / 100.0;
            Q_EMIT offsetChanged(p.offset);
            _offsetUpdate();
        }

        void AudioOffsetWidget::_spinBoxCallback(double value)
        {
            TLRENDER_P();
            p.offset = value;
            Q_EMIT offsetChanged(p.offset);
            _offsetUpdate();
        }

        void AudioOffsetWidget::_resetCallback()
        {
            TLRENDER_P();
            p.offset = 0.0;
            Q_EMIT offsetChanged(p.offset);
            _offsetUpdate();
        }

        void AudioOffsetWidget::_offsetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.spinBox);
                p.spinBox->setValue(p.offset);
            }
            {
                QSignalBlocker signalBlocker(p.slider);
                p.slider->setValue(p.offset * 100);
            }
        }

        struct AudioTool::Private
        {
            AudioOffsetWidget* offsetWidget = nullptr;
        };

        AudioTool::AudioTool(QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.offsetWidget = new AudioOffsetWidget;

            addBellows(tr("Sync Offset"), p.offsetWidget);
            addStretch();

            connect(
                p.offsetWidget,
                SIGNAL(offsetChanged(double)),
                SIGNAL(audioOffsetChanged(double)));
        }

        void AudioTool::setAudioOffset(double value)
        {
            TLRENDER_P();
            p.offsetWidget->setAudioOffset(value);
        }
    }
}
