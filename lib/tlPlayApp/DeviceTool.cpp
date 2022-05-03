// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/DeviceTool.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/DeviceModel.h>
#include <tlPlayApp/SettingsObject.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QFormLayout>
#include <QSignalBlocker>

namespace tl
{
    namespace play
    {
        struct DeviceTool::Private
        {
            App* app = nullptr;
            std::shared_ptr<observer::ValueObserver<DeviceModelData> > dataObserver;
            QComboBox* deviceComboBox = nullptr;
            QComboBox* displayModeComboBox = nullptr;
        };

        DeviceTool::DeviceTool(
            App* app,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.deviceComboBox = new QComboBox;

            p.displayModeComboBox = new QComboBox;

            auto layout = new QFormLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addRow(tr("Device:"), p.deviceComboBox);
            layout->addRow(tr("Display mode:"), p.displayModeComboBox);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget, 1);

            connect(
                p.deviceComboBox,
                SIGNAL(activated(int)),
                SLOT(_deviceCallback(int)));

            connect(
                p.displayModeComboBox,
                SIGNAL(activated(int)),
                SLOT(_displayModeCallback(int)));

            p.dataObserver = observer::ValueObserver<DeviceModelData>::create(
                app->deviceModel()->observeData(),
                [this](const DeviceModelData& value)
                {
                    {
                        QSignalBlocker blocker(_p->deviceComboBox);
                        _p->deviceComboBox->clear();
                        for (const auto& i : value.devices)
                        {
                            _p->deviceComboBox->addItem(QString::fromUtf8(i.c_str()));
                        }
                        _p->deviceComboBox->setCurrentIndex(value.deviceIndex);
                    }
                    {
                        QSignalBlocker blocker(_p->displayModeComboBox);
                        _p->displayModeComboBox->clear();
                        for (const auto& i : value.displayModes)
                        {
                            _p->displayModeComboBox->addItem(QString::fromUtf8(i.c_str()));
                        }
                        _p->displayModeComboBox->setCurrentIndex(value.displayModeIndex);
                    }
                });
        }

        DeviceTool::~DeviceTool()
        {}

        void DeviceTool::_deviceCallback(int index)
        {
            TLRENDER_P();
            p.app->deviceModel()->setDevice(index);
        }

        void DeviceTool::_displayModeCallback(int index)
        {
            TLRENDER_P();
            p.app->deviceModel()->setDisplayMode(index);
        }
    }
}
