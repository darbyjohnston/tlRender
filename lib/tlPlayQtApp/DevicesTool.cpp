// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/DevicesTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/FloatEditSlider.h>

#if defined(TLRENDER_BMD)
#include <tlDevice/BMDDevicesModel.h>
#include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSignalBlocker>

#include <sstream>

namespace tl
{
    namespace play_qt
    {
        struct DevicesTool::Private
        {
            App* app = nullptr;

#if defined(TLRENDER_BMD)
            QCheckBox* enabledCheckBox = nullptr;
            QComboBox* deviceComboBox = nullptr;
            QComboBox* displayModeComboBox = nullptr;
            QComboBox* pixelTypeComboBox = nullptr;
            QCheckBox* _444SDIVideoOutputCheckBox = nullptr;
            QComboBox* videoLevelsComboBox = nullptr;

            std::shared_ptr<dtk::ValueObserver<bmd::DevicesModelData> > dataObserver;
#endif // TLRENDER_BMD
        };

        DevicesTool::DevicesTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            DTK_P();

            p.app = app;

#if defined(TLRENDER_BMD)
            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.deviceComboBox = new QComboBox;
            p.displayModeComboBox = new QComboBox;
            p.pixelTypeComboBox = new QComboBox;

            p._444SDIVideoOutputCheckBox = new QCheckBox(tr("444 SDI video output"));

            p.videoLevelsComboBox = new QComboBox;
            for (const auto& i : dtk::getVideoLevelsLabels())
            {
                p.videoLevelsComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("Device:"), p.deviceComboBox);
            layout->addRow(tr("Display mode:"), p.displayModeComboBox);
            layout->addRow(tr("Pixel type:"), p.pixelTypeComboBox);
            layout->addRow(p._444SDIVideoOutputCheckBox);
            layout->addRow(tr("Video levels:"), p.videoLevelsComboBox);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addBellows(tr("Output"), widget);

            addStretch();

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    _p->app->bmdDevicesModel()->setDeviceEnabled(value);
                });

            connect(
                p.deviceComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->bmdDevicesModel()->setDeviceIndex(value);
                });
            connect(
                p.displayModeComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->bmdDevicesModel()->setDisplayModeIndex(value);
                });
            connect(
                p.pixelTypeComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->bmdDevicesModel()->setPixelTypeIndex(value);
                });

            connect(
                p._444SDIVideoOutputCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    auto options = _p->app->bmdDevicesModel()->observeData()->get().boolOptions;
                    options[bmd::Option::_444SDIVideoOutput] = value;
                    _p->app->bmdDevicesModel()->setBoolOptions(options);
                });

            connect(
                p.videoLevelsComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->bmdDevicesModel()->setVideoLevels(static_cast<dtk::VideoLevels>(value));
                });

            p.dataObserver = dtk::ValueObserver<bmd::DevicesModelData>::create(
                app->bmdDevicesModel()->observeData(),
                [this](const bmd::DevicesModelData& value)
                {
                    DTK_P();
                    {
                        QSignalBlocker blocker(p.enabledCheckBox);
                        p.enabledCheckBox->setChecked(value.deviceEnabled);
                    }
                    {
                        QSignalBlocker blocker(p.deviceComboBox);
                        p.deviceComboBox->clear();
                        for (const auto& i : value.devices)
                        {
                            p.deviceComboBox->addItem(QString::fromUtf8(i.c_str()));
                        }
                        p.deviceComboBox->setCurrentIndex(value.deviceIndex);
                    }
                    {
                        QSignalBlocker blocker(p.displayModeComboBox);
                        p.displayModeComboBox->clear();
                        for (const auto& i : value.displayModes)
                        {
                            p.displayModeComboBox->addItem(QString::fromUtf8(i.c_str()));
                        }
                        p.displayModeComboBox->setCurrentIndex(value.displayModeIndex);
                    }
                    {
                        QSignalBlocker blocker(p.pixelTypeComboBox);
                        p.pixelTypeComboBox->clear();
                        for (const auto& i : value.pixelTypes)
                        {
                            std::stringstream ss;
                            ss << i;
                            p.pixelTypeComboBox->addItem(QString::fromUtf8(ss.str().c_str()));
                        }
                        p.pixelTypeComboBox->setCurrentIndex(value.pixelTypeIndex);
                    }
                    {
                        QSignalBlocker blocker(p._444SDIVideoOutputCheckBox);
                        const auto i = value.boolOptions.find(bmd::Option::_444SDIVideoOutput);
                        p._444SDIVideoOutputCheckBox->setChecked(i != value.boolOptions.end() ? i->second : false);
                    }
                    {
                        QSignalBlocker blocker(p.videoLevelsComboBox);
                        p.videoLevelsComboBox->setCurrentIndex(static_cast<size_t>(value.videoLevels));
                    }
                });
#endif // TLRENDER_BMD
        }

        DevicesTool::~DevicesTool()
        {}

        DevicesDockWidget::DevicesDockWidget(
            DevicesTool* devicesTool,
            QWidget* parent)
        {
            setObjectName("DevicesTool");
            setWindowTitle(tr("Devices"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Devices"));
            dockTitleBar->setIcon(QIcon(":/Icons/Devices.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(devicesTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Devices.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F6));
            toggleViewAction()->setToolTip(tr("Show devices"));
        }
    }
}
