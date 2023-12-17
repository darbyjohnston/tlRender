// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/BMDDevicesModel.h>

#include <tlDevice/BMDDeviceSystem.h>

#include <tlCore/Context.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play_qt
    {
        bool BMDDevicesModelData::operator == (const BMDDevicesModelData& other) const
        {
            return
                devices == other.devices &&
                deviceIndex == other.deviceIndex &&
                displayModes == other.displayModes &&
                displayModeIndex == other.displayModeIndex &&
                pixelTypes == other.pixelTypes &&
                pixelTypeIndex == other.pixelTypeIndex &&
                deviceEnabled == other.deviceEnabled &&
                videoLevels == other.videoLevels &&
                hdrMode == other.hdrMode &&
                hdrData == other.hdrData;
        }

        struct BMDDevicesModel::Private
        {
            std::vector<device::DeviceInfo> deviceInfo;
            int deviceIndex = 0;
            int displayModeIndex = 0;
            int pixelTypeIndex = 0;
            bool deviceEnabled = true;
            image::VideoLevels videoLevels = image::VideoLevels::LegalRange;
            device::HDRMode hdrMode = device::HDRMode::FromFile;
            image::HDRData hdrData;
            std::shared_ptr<observer::Value<BMDDevicesModelData> > data;
            std::shared_ptr<observer::ListObserver<device::DeviceInfo> > deviceInfoObserver;
        };

        void BMDDevicesModel::_init(
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.data = observer::Value<BMDDevicesModelData>::create();

            _update();

            if (auto deviceSystem = context->getSystem<device::BMDDeviceSystem>())
            {
                p.deviceInfoObserver = observer::ListObserver<device::DeviceInfo>::create(
                    deviceSystem->observeDeviceInfo(),
                    [this](const std::vector<device::DeviceInfo>& value)
                    {
                        _p->deviceInfo = value;
                        _update();
                    });
            }
        }

        BMDDevicesModel::BMDDevicesModel() :
            _p(new Private)
        {}

        BMDDevicesModel::~BMDDevicesModel()
        {}

        std::shared_ptr<BMDDevicesModel> BMDDevicesModel::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<BMDDevicesModel>(new BMDDevicesModel);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IValue<BMDDevicesModelData> > BMDDevicesModel::observeData() const
        {
            return _p->data;
        }

        void BMDDevicesModel::setDeviceIndex(int index)
        {
            TLRENDER_P();
            if (index == p.deviceIndex)
                return;
            p.deviceIndex = index;
            _update();
        }

        void BMDDevicesModel::setDisplayModeIndex(int index)
        {
            TLRENDER_P();
            if (index == p.displayModeIndex)
                return;
            p.displayModeIndex = index;
            _update();
        }

        void BMDDevicesModel::setPixelTypeIndex(int index)
        {
            TLRENDER_P();
            if (index == p.pixelTypeIndex)
                return;
            p.pixelTypeIndex = index;
            _update();
        }

        void BMDDevicesModel::setDeviceEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.deviceEnabled)
                return;
            p.deviceEnabled = value;
            _update();
        }

        void BMDDevicesModel::setVideoLevels(image::VideoLevels value)
        {
            TLRENDER_P();
            if (value == p.videoLevels)
                return;
            p.videoLevels = value;
            _update();
        }

        void BMDDevicesModel::setHDRMode(device::HDRMode value)
        {
            TLRENDER_P();
            if (value == p.hdrMode)
                return;
            p.hdrMode = value;
            _update();
        }

        void BMDDevicesModel::setHDRData(const image::HDRData& value)
        {
            TLRENDER_P();
            if (value == p.hdrData)
                return;
            p.hdrData = value;
            _update();
        }

        void BMDDevicesModel::_update()
        {
            TLRENDER_P();

            BMDDevicesModelData data;

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

            data.pixelTypes.push_back(device::PixelType::None);
            if (!p.deviceInfo.empty() && p.deviceIndex >= 1 && (p.deviceIndex - 1) < p.deviceInfo.size())
            {
                for (const auto& i : p.deviceInfo[p.deviceIndex - 1].pixelTypes)
                {
                    data.pixelTypes.push_back(i);
                }
                data.pixelTypeIndex = p.pixelTypeIndex;
            }

            data.deviceEnabled = p.deviceEnabled;

            data.videoLevels = p.videoLevels;

            data.hdrMode = p.hdrMode;
            data.hdrData = p.hdrData;

            p.data->setIfChanged(data);
        }
    }
}
