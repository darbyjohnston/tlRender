// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/AudioTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlPlay/AudioModel.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <QAction>
#include <QBoxLayout>
#include <QComboBox>

namespace tl
{
    namespace play_qt
    {
        struct AudioDeviceWidget::Private
        {
            QComboBox* deviceComboBox = nullptr;

            std::shared_ptr<observer::ListObserver<std::string> > audioDevicesObserver;
            std::shared_ptr<observer::ValueObserver<int> > audioDeviceObserver;
        };

        AudioDeviceWidget::AudioDeviceWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.deviceComboBox = new QComboBox;

            auto layout = new QVBoxLayout;
            layout->addWidget(p.deviceComboBox);
            setLayout(layout);

            connect(
                p.deviceComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [app](int value)
                {
                    app->audioModel()->setDevice(value);
                });

            p.audioDevicesObserver = observer::ListObserver<std::string>::create(
                app->audioModel()->observeDevices(),
                [this](const std::vector<std::string>& devices)
                {
                    const QSignalBlocker blocker(_p->deviceComboBox);
                    _p->deviceComboBox->clear();
                    for (const auto& device : devices)
                    {
                        _p->deviceComboBox->addItem(QString::fromUtf8(device.c_str()));
                    }
                });
            p.audioDeviceObserver = observer::ValueObserver<int>::create(
                app->audioModel()->observeDevice(),
                [this](int value)
                {
                    const QSignalBlocker blocker(_p->deviceComboBox);
                    _p->deviceComboBox->setCurrentIndex(value);
                });
        }

        AudioDeviceWidget::~AudioDeviceWidget()
        {}

        struct AudioOffsetWidget::Private
        {
            qtwidget::FloatEditSlider* slider = nullptr;

            std::shared_ptr<observer::ValueObserver<double> > syncOffsetObserver;
        };

        AudioOffsetWidget::AudioOffsetWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.slider = new qtwidget::FloatEditSlider;
            p.slider->setRange(math::FloatRange(-1.F, 1.F));
            p.slider->setDefaultValue(0.F);

            auto layout = new QVBoxLayout;
            layout->addWidget(p.slider);
            setLayout(layout);

            connect(
                p.slider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    app->audioModel()->setSyncOffset(value);
                });

            p.syncOffsetObserver = observer::ValueObserver<double>::create(
                app->audioModel()->observeSyncOffset(),
                [this](double value)
                {
                    QSignalBlocker signalBlocker(_p->slider);
                    _p->slider->setValue(value);
                });
        }

        AudioOffsetWidget::~AudioOffsetWidget()
        {}

        struct AudioTool::Private
        {
            AudioDeviceWidget* deviceWidget = nullptr;
            AudioOffsetWidget* offsetWidget = nullptr;
        };

        AudioTool::AudioTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.deviceWidget = new AudioDeviceWidget(app);
            p.offsetWidget = new AudioOffsetWidget(app);

            addBellows(tr("Output Device"), p.deviceWidget);
            addBellows(tr("Sync Offset"), p.offsetWidget);
            addStretch();
        }

        AudioTool::~AudioTool()
        {}

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
