// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/DeviceModel.h>

#include <tlDevice/IDeviceSystem.h>

#include <tlCore/Context.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play
    {
        bool DeviceModelData::operator == (const DeviceModelData& other) const
        {
            return
                devices == other.devices &&
                deviceIndex == other.deviceIndex &&
                displayModes == other.displayModes &&
                displayModeIndex == other.displayModeIndex;
        }

        struct DeviceModel::Private
        {
            std::vector<device::DeviceInfo> deviceInfo;
            int deviceIndex = 0;
            int displayModeIndex = 0;
            std::shared_ptr<observer::Value<DeviceModelData> > data;
            std::shared_ptr<observer::ListObserver<device::DeviceInfo> > deviceInfoObserver;
        };

        void DeviceModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.data = observer::Value<DeviceModelData>::create();

            _deviceInfoUpdate();

            if (auto deviceSystem = context->getSystem<device::IDeviceSystem>())
            {
                p.deviceInfoObserver = observer::ListObserver<device::DeviceInfo>::create(
                    deviceSystem->observeDeviceInfo(),
                    [this](const std::vector<device::DeviceInfo>& value)
                    {
                        _p->deviceInfo = value;
                        _deviceInfoUpdate();
                    });
            }
        }

        DeviceModel::DeviceModel() :
            _p(new Private)
        {}

        DeviceModel::~DeviceModel()
        {}

        std::shared_ptr<DeviceModel> DeviceModel::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<DeviceModel>(new DeviceModel);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IValue<DeviceModelData> > DeviceModel::observeData() const
        {
            return _p->data;
        }

        void DeviceModel::setDevice(int index)
        {
            TLRENDER_P();
            if (index == p.deviceIndex)
                return;
            p.deviceIndex = index;
            _deviceInfoUpdate();
        }

        void DeviceModel::setDisplayMode(int index)
        {
            TLRENDER_P();
            if (index == p.displayModeIndex)
                return;
            p.displayModeIndex = index;
            _deviceInfoUpdate();
        }

        void DeviceModel::_deviceInfoUpdate()
        {
            TLRENDER_P();
            DeviceModelData data;
            data.devices.push_back("None");
            for (const auto& i : p.deviceInfo)
            {
                data.devices.push_back(i.name);
            }
            data.deviceIndex = p.deviceIndex;
            data.displayModes.push_back("None");
            if (!p.deviceInfo.empty() && p.deviceIndex >= 1 && (p.deviceIndex - 1) < p.deviceInfo.size())
            {
                for (const auto& i : p.deviceInfo[p.deviceIndex - 1].displayModes)
                {
                    data.displayModes.push_back(i.name);
                }
                data.displayModeIndex = p.displayModeIndex;
            }
            p.data->setIfChanged(data);
        }
    }
}
