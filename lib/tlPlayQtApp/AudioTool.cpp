// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
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
            std::vector<audio::DeviceID> devices;

            QComboBox* deviceComboBox = nullptr;

            std::shared_ptr<dtk::ListObserver<audio::DeviceID> > audioDevicesObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceID> > audioDeviceObserver;
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
                [this, app](int value)
                {
                    if (value >= 0 && value < _p->devices.size())
                    {
                        app->audioModel()->setDevice(
                            0 == value ? audio::DeviceID() : _p->devices[value]);
                    }
                });

            p.audioDevicesObserver = dtk::ListObserver<audio::DeviceID>::create(
                app->audioModel()->observeDevices(),
                [this](const std::vector<audio::DeviceID>& devices)
                {
                    _p->devices.clear();
                    _p->devices.push_back(audio::DeviceID());
                    _p->devices.insert(_p->devices.end(), devices.begin(), devices.end());
                    std::vector<std::string> names;
                    names.push_back("Default");
                    for (const auto& device : devices)
                    {
                        names.push_back(device.name);
                    }
                    const QSignalBlocker blocker(_p->deviceComboBox);
                    _p->deviceComboBox->clear();
                    for (const auto& name : names)
                    {
                        _p->deviceComboBox->addItem(QString::fromUtf8(name.c_str()));
                    }
                });

            p.audioDeviceObserver = dtk::ValueObserver<audio::DeviceID>::create(
                app->audioModel()->observeDevice(),
                [this](const audio::DeviceID &value)
                {
                    int index = 0;
                    const auto i = std::find(_p->devices.begin(), _p->devices.end(), value);
                    if (i != _p->devices.end())
                    {
                        index = i - _p->devices.begin();
                    }
                    const QSignalBlocker blocker(_p->deviceComboBox);
                    _p->deviceComboBox->setCurrentIndex(index);
                });
        }

        AudioDeviceWidget::~AudioDeviceWidget()
        {}

        struct AudioOffsetWidget::Private
        {
            qtwidget::FloatEditSlider* slider = nullptr;

            std::shared_ptr<dtk::ValueObserver<double> > syncOffsetObserver;
        };

        AudioOffsetWidget::AudioOffsetWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.slider = new qtwidget::FloatEditSlider;
            p.slider->setRange(dtk::RangeF(-1.F, 1.F));
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

            p.syncOffsetObserver = dtk::ValueObserver<double>::create(
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
