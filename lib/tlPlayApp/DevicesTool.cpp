// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/DevicesTool.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/DockTitleBar.h>
#include <tlPlayApp/DevicesModel.h>

#include <tlQtWidget/FloatSlider.h>
#include <tlQtWidget/Spacer.h>

#include <tlQt/OutputDevice.h>

#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSignalBlocker>

#include <sstream>

namespace tl
{
    namespace play
    {
        namespace
        {
            QDoubleSpinBox* createPrimariesSpinBox()
            {
                QDoubleSpinBox* out = new QDoubleSpinBox;
                out = new QDoubleSpinBox;
                out->setRange(0.0, 1.0);
                out->setSingleStep(0.01);
                return out;
            }
        }

        struct DevicesTool::Private
        {
            App* app = nullptr;
            std::shared_ptr<observer::ValueObserver<DevicesModelData> > dataObserver;
            QComboBox* deviceComboBox = nullptr;
            QComboBox* displayModeComboBox = nullptr;
            QComboBox* pixelTypeComboBox = nullptr;
            QComboBox* hdrModeComboBox = nullptr;
            std::pair<QDoubleSpinBox*, QDoubleSpinBox*> redPrimariesSpinBoxes =
                std::make_pair(nullptr, nullptr);
            std::pair<QDoubleSpinBox*, QDoubleSpinBox*> greenPrimariesSpinBoxes =
                std::make_pair(nullptr, nullptr);
            std::pair<QDoubleSpinBox*, QDoubleSpinBox*> bluePrimariesSpinBoxes =
                std::make_pair(nullptr, nullptr);
            std::pair<QDoubleSpinBox*, QDoubleSpinBox*> whitePrimariesSpinBoxes =
                std::make_pair(nullptr, nullptr);
            std::pair<QDoubleSpinBox*, QDoubleSpinBox*> masteringLuminanceSpinBoxes =
                std::make_pair(nullptr, nullptr);
            qtwidget::FloatSlider* maxCLLSlider = nullptr;
            qtwidget::FloatSlider* maxFALLSlider = nullptr;
        };

        DevicesTool::DevicesTool(
            App* app,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.deviceComboBox = new QComboBox;

            p.displayModeComboBox = new QComboBox;

            p.pixelTypeComboBox = new QComboBox;

            p.hdrModeComboBox = new QComboBox;

            p.redPrimariesSpinBoxes.first = createPrimariesSpinBox();
            p.redPrimariesSpinBoxes.second = createPrimariesSpinBox();

            p.greenPrimariesSpinBoxes.first = createPrimariesSpinBox();
            p.greenPrimariesSpinBoxes.second = createPrimariesSpinBox();

            p.bluePrimariesSpinBoxes.first = createPrimariesSpinBox();
            p.bluePrimariesSpinBoxes.second = createPrimariesSpinBox();

            p.whitePrimariesSpinBoxes.first = createPrimariesSpinBox();
            p.whitePrimariesSpinBoxes.second = createPrimariesSpinBox();

            p.masteringLuminanceSpinBoxes.first = new QDoubleSpinBox;
            p.masteringLuminanceSpinBoxes.first->setRange(0.0, 10000.0);
            p.masteringLuminanceSpinBoxes.second = new QDoubleSpinBox;
            p.masteringLuminanceSpinBoxes.second->setRange(0.0, 10000.0);

            p.maxCLLSlider = new qtwidget::FloatSlider;
            p.maxCLLSlider->setRange(math::FloatRange(0.F, 10000.F));

            p.maxFALLSlider = new qtwidget::FloatSlider;
            p.maxFALLSlider->setRange(math::FloatRange(0.F, 10000.F));

            auto layout = new QFormLayout;
            layout->addRow(tr("Name:"), p.deviceComboBox);
            layout->addRow(tr("Display mode:"), p.displayModeComboBox);
            layout->addRow(tr("Pixel type:"), p.pixelTypeComboBox);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addBellows(tr("Output"), widget);

            layout = new QFormLayout;
            layout->addRow(tr("Mode:"), p.hdrModeComboBox);
            layout->addRow(new qtwidget::Spacer(Qt::Vertical));
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.redPrimariesSpinBoxes.first);
            hLayout->addWidget(p.redPrimariesSpinBoxes.second);
            layout->addRow(tr("Red primaries:"), hLayout);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.greenPrimariesSpinBoxes.first);
            hLayout->addWidget(p.greenPrimariesSpinBoxes.second);
            layout->addRow(tr("Green primaries:"), hLayout);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.bluePrimariesSpinBoxes.first);
            hLayout->addWidget(p.bluePrimariesSpinBoxes.second);
            layout->addRow(tr("Blue primaries:"), hLayout);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.whitePrimariesSpinBoxes.first);
            hLayout->addWidget(p.whitePrimariesSpinBoxes.second);
            layout->addRow(tr("White primaries:"), hLayout);
            layout->addRow(new qtwidget::Spacer(Qt::Vertical));
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.masteringLuminanceSpinBoxes.first);
            hLayout->addWidget(p.masteringLuminanceSpinBoxes.second);
            layout->addRow(tr("Mastering luminance:"), hLayout);
            layout->addRow(tr("Maximum CLL:"), p.maxCLLSlider);
            layout->addRow(tr("Maximum FALL:"), p.maxFALLSlider);
            widget = new QWidget;
            widget->setLayout(layout);
            addBellows(tr("HDR"), widget);

            addStretch();

            connect(
                p.deviceComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->devicesModel()->setDeviceIndex(value);
                });

            connect(
                p.displayModeComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->devicesModel()->setDisplayModeIndex(value);
                });

            connect(
                p.pixelTypeComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->devicesModel()->setPixelTypeIndex(value);
                });

            connect(
                p.hdrModeComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->devicesModel()->setHDRMode(static_cast<device::HDRMode>(value));
                });

            connect(
                p.redPrimariesSpinBoxes.first,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.redPrimaries.x = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });
            connect(
                p.redPrimariesSpinBoxes.second,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.redPrimaries.y = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });

            connect(
                p.greenPrimariesSpinBoxes.first,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.greenPrimaries.x = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });
            connect(
                p.greenPrimariesSpinBoxes.second,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.greenPrimaries.y = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });

            connect(
                p.bluePrimariesSpinBoxes.first,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.bluePrimaries.x = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });
            connect(
                p.bluePrimariesSpinBoxes.second,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.bluePrimaries.y = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });

            connect(
                p.whitePrimariesSpinBoxes.first,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.whitePrimaries.x = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });
            connect(
                p.whitePrimariesSpinBoxes.second,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.whitePrimaries.y = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });

            connect(
                p.masteringLuminanceSpinBoxes.first,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.displayMasteringLuminance = math::FloatRange(value, hdrData.displayMasteringLuminance.getMax());
                    _p->app->devicesModel()->setHDRData(hdrData);
                });
            connect(
                p.masteringLuminanceSpinBoxes.second,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.displayMasteringLuminance = math::FloatRange(hdrData.displayMasteringLuminance.getMin(), value);
                    _p->app->devicesModel()->setHDRData(hdrData);
                });

            connect(
                p.maxCLLSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.maxCLL = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });
            connect(
                p.maxFALLSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    auto hdrData = _p->app->devicesModel()->observeData()->get().hdrData;
                    hdrData.maxFALL = value;
                    _p->app->devicesModel()->setHDRData(hdrData);
                });

            p.dataObserver = observer::ValueObserver<DevicesModelData>::create(
                app->devicesModel()->observeData(),
                [this](const DevicesModelData& value)
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
                    {
                        QSignalBlocker blocker(_p->pixelTypeComboBox);
                        _p->pixelTypeComboBox->clear();
                        for (const auto& i : value.pixelTypes)
                        {
                            std::stringstream ss;
                            ss << i;
                            _p->pixelTypeComboBox->addItem(QString::fromUtf8(ss.str().c_str()));
                        }
                        _p->pixelTypeComboBox->setCurrentIndex(value.pixelTypeIndex);
                    }
                    {
                        QSignalBlocker blocker(_p->hdrModeComboBox);
                        _p->hdrModeComboBox->clear();
                        for (const auto& i : device::getHDRModeLabels())
                        {
                            _p->hdrModeComboBox->addItem(QString::fromUtf8(i.c_str()));
                        }
                        _p->hdrModeComboBox->setCurrentIndex(static_cast<int>(value.hdrMode));
                    }
                    {
                        QSignalBlocker blocker(_p->redPrimariesSpinBoxes.first);
                        _p->redPrimariesSpinBoxes.first->setValue(value.hdrData.redPrimaries.x);
                        _p->redPrimariesSpinBoxes.first->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->redPrimariesSpinBoxes.second);
                        _p->redPrimariesSpinBoxes.second->setValue(value.hdrData.redPrimaries.y);
                        _p->redPrimariesSpinBoxes.second->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->greenPrimariesSpinBoxes.first);
                        _p->greenPrimariesSpinBoxes.first->setValue(value.hdrData.greenPrimaries.x);
                        _p->greenPrimariesSpinBoxes.first->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->greenPrimariesSpinBoxes.second);
                        _p->greenPrimariesSpinBoxes.second->setValue(value.hdrData.greenPrimaries.y);
                        _p->greenPrimariesSpinBoxes.second->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->bluePrimariesSpinBoxes.first);
                        _p->bluePrimariesSpinBoxes.first->setValue(value.hdrData.bluePrimaries.x);
                        _p->bluePrimariesSpinBoxes.first->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->bluePrimariesSpinBoxes.second);
                        _p->bluePrimariesSpinBoxes.second->setValue(value.hdrData.bluePrimaries.y);
                        _p->bluePrimariesSpinBoxes.second->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->whitePrimariesSpinBoxes.first);
                        _p->whitePrimariesSpinBoxes.first->setValue(value.hdrData.whitePrimaries.x);
                        _p->whitePrimariesSpinBoxes.first->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->whitePrimariesSpinBoxes.second);
                        _p->whitePrimariesSpinBoxes.second->setValue(value.hdrData.whitePrimaries.y);
                        _p->whitePrimariesSpinBoxes.second->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->masteringLuminanceSpinBoxes.first);
                        _p->masteringLuminanceSpinBoxes.first->setValue(value.hdrData.displayMasteringLuminance.getMin());
                        _p->masteringLuminanceSpinBoxes.first->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->masteringLuminanceSpinBoxes.second);
                        _p->masteringLuminanceSpinBoxes.second->setValue(value.hdrData.displayMasteringLuminance.getMax());
                        _p->masteringLuminanceSpinBoxes.second->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->maxCLLSlider);
                        _p->maxCLLSlider->setValue(value.hdrData.maxCLL);
                        _p->maxCLLSlider->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                    {
                        QSignalBlocker blocker(_p->maxFALLSlider);
                        _p->maxFALLSlider->setValue(value.hdrData.maxFALL);
                        _p->maxFALLSlider->setEnabled(device::HDRMode::Custom == value.hdrMode);
                    }
                });
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
            dockTitleBar->setText(tr("DEVICES"));
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
