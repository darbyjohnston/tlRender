// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/AudioTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlPlay/AudioModel.h>

#include <tlQtWidget/FloatEditSlider.h>
#include <tlQtWidget/IntEditSlider.h>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>

namespace tl
{
    namespace play_qt
    {
        struct AudioTool::Private
        {
            std::vector<audio::DeviceID> devices;

            QComboBox* deviceComboBox = nullptr;
            qtwidget::IntEditSlider* volumeSlider = nullptr;
            QCheckBox* muteCheckBox = nullptr;
            qtwidget::FloatEditSlider* syncOffsetSlider = nullptr;
            QFormLayout* layout = nullptr;

            std::shared_ptr<dtk::ListObserver<audio::DeviceID> > devicesObserver;
            std::shared_ptr<dtk::ValueObserver<audio::DeviceID> > deviceObserver;
            std::shared_ptr<dtk::ValueObserver<float> > volumeObserver;
            std::shared_ptr<dtk::ValueObserver<bool> > muteObserver;
            std::shared_ptr<dtk::ValueObserver<double> > syncOffsetObserver;
        };

        AudioTool::AudioTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            DTK_P();

            p.deviceComboBox = new QComboBox;

            p.volumeSlider = new qtwidget::IntEditSlider;
            p.volumeSlider->setRange(dtk::RangeI(0, 100));
            p.volumeSlider->setSingleStep(1);
            p.volumeSlider->setPageStep(10);

            p.muteCheckBox = new QCheckBox(tr("Mute"));

            p.syncOffsetSlider = new qtwidget::FloatEditSlider;
            p.syncOffsetSlider->setRange(dtk::RangeF(-1.F, 1.F));
            p.syncOffsetSlider->setDefaultValue(0.F);

            p.layout = new QFormLayout;
            p.layout->addRow(tr("Device:"), p.deviceComboBox);
            p.layout->addRow(tr("Volume:"), p.volumeSlider);
            p.layout->addRow(p.muteCheckBox);
            p.layout->addRow(tr("Sync offset:"), p.syncOffsetSlider);

            auto widget = new QWidget;
            widget->setLayout(p.layout);
            addWidget(widget);

            addStretch();

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

            connect(
                p.volumeSlider,
                &qtwidget::IntEditSlider::valueChanged,
                [app](int value)
                {
                    app->audioModel()->setVolume(value / 100.F);
                });

            connect(
                p.muteCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    app->audioModel()->setMute(value);
                });

            connect(
                p.syncOffsetSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    app->audioModel()->setSyncOffset(value);
                });

            p.devicesObserver = dtk::ListObserver<audio::DeviceID>::create(
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

            p.deviceObserver = dtk::ValueObserver<audio::DeviceID>::create(
                app->audioModel()->observeDevice(),
                [this](const audio::DeviceID& value)
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

            p.volumeObserver = dtk::ValueObserver<float>::create(
                app->audioModel()->observeVolume(),
                [this](float value)
                {
                    _p->volumeSlider->setValue(std::roundf(value * 100.F));
                });

            p.muteObserver = dtk::ValueObserver<bool>::create(
                app->audioModel()->observeMute(),
                [this](bool value)
                {
                    _p->muteCheckBox->setChecked(value);
                });

            p.syncOffsetObserver = dtk::ValueObserver<double>::create(
                app->audioModel()->observeSyncOffset(),
                [this](double value)
                {
                    QSignalBlocker signalBlocker(_p->syncOffsetSlider);
                    _p->syncOffsetSlider->setValue(value);
                });
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
