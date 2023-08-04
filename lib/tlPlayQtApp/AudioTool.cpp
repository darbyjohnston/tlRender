// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/AudioTool.h>

#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <QAction>
#include <QBoxLayout>

namespace tl
{
    namespace play_qt
    {
        struct AudioOffsetWidget::Private
        {
            double offset = 0.0;
            qtwidget::FloatEditSlider* slider = nullptr;
        };

        AudioOffsetWidget::AudioOffsetWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.slider = new qtwidget::FloatEditSlider;
            p.slider->setRange(math::FloatRange(-1.F, 1.F));
            p.slider->setDefaultValue(0.F);

            auto layout = new QVBoxLayout;
            layout->addWidget(p.slider);
            layout->addStretch();
            setLayout(layout);

            connect(
                p.slider,
                &qtwidget::FloatEditSlider::valueChanged,
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

        AudioTool::AudioTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
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

        AudioDockWidget::AudioDockWidget(
            AudioTool* audioTool,
            QWidget* parent)
        {
            setObjectName("AudioTool");
            setWindowTitle(tr("Audio"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Audio"));
            dockTitleBar->setIcon(QIcon(":/Icons/Audio.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(audioTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Audio.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F5));
            toggleViewAction()->setToolTip(tr("Show audio controls"));
        }
    }
}
