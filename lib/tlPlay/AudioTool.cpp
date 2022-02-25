// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/AudioTool.h>

#include <tlQWidget/FloatSlider.h>

#include <QBoxLayout>

namespace tl
{
    namespace play
    {
        struct AudioOffsetWidget::Private
        {
            double offset = 0.0;
            qwidget::FloatSlider* slider = nullptr;
        };

        AudioOffsetWidget::AudioOffsetWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.slider = new qwidget::FloatSlider;
            p.slider->setRange(math::FloatRange(-1.F, 1.F));
            p.slider->setDefaultValue(0.F);

            auto layout = new QVBoxLayout;
            layout->addWidget(p.slider);
            layout->addStretch();
            setLayout(layout);

            connect(
                p.slider,
                &qwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    _p->offset = value;
                    Q_EMIT audioOffsetChanged(_p->offset);
                });
        }

        AudioOffsetWidget::~AudioOffsetWidget()
        {}

        void AudioOffsetWidget::setAudioOffset(double value)
        {
            TLRENDER_P();
            p.offset = value;
            _offsetUpdate();
        }

        void AudioOffsetWidget::_offsetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.slider);
                p.slider->setValue(p.offset);
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
                SIGNAL(audioOffsetChanged(double)),
                SIGNAL(audioOffsetChanged(double)));
        }

        AudioTool::~AudioTool()
        {}

        void AudioTool::setAudioOffset(double value)
        {
            TLRENDER_P();
            p.offsetWidget->setAudioOffset(value);
        }
    }
}
