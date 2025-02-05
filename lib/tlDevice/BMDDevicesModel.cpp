// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDDevicesModel.h>

#include <tlDevice/BMDSystem.h>

#include <dtk/core/Context.h>

namespace tl
{
    namespace bmd
    {
        bool DevicesModelData::operator == (const DevicesModelData& other) const
        {
            return
                devices == other.devices &&
                deviceIndex == other.deviceIndex &&
                displayModes == other.displayModes &&
                displayModeIndex == other.displayModeIndex &&
                pixelTypes == other.pixelTypes &&
                pixelTypeIndex == other.pixelTypeIndex &&
                deviceEnabled == other.deviceEnabled &&
                boolOptions == other.boolOptions &&
                videoLevels == other.videoLevels &&
                hdrMode == other.hdrMode &&
                hdrData == other.hdrData;
        }

        struct DevicesModel::Private
        {
            std::vector<DeviceInfo> deviceInfo;
            int deviceIndex = 0;
            int displayModeIndex = 0;
            int pixelTypeIndex = 0;
            bool deviceEnabled = true;
            BoolOptions boolOptions;
            dtk::VideoLevels videoLevels = dtk::VideoLevels::LegalRange;
            HDRMode hdrMode = HDRMode::FromFile;
            image::HDRData hdrData;
            std::shared_ptr<dtk::ObservableValue<DevicesModelData> > data;
            std::shared_ptr<dtk::ListObserver<DeviceInfo> > deviceInfoObserver;
        };

        void DevicesModel::_init(
            const std::shared_ptr<dtk::Context>& context)
        {
            DTK_P();

            p.data = dtk::ObservableValue<DevicesModelData>::create();

            _update();

            if (auto system = context->getSystem<System>())
            {
                p.deviceInfoObserver = dtk::ListObserver<DeviceInfo>::create(
                    system->observeDeviceInfo(),
                    [this](const std::vector<DeviceInfo>& value)
                    {
                        _p->deviceInfo = value;
                        _update();
                    });
            }
        }

        DevicesModel::DevicesModel() :
            _p(new Private)
        {}

        DevicesModel::~DevicesModel()
        {}

        std::shared_ptr<DevicesModel> DevicesModel::create(
            const std::shared_ptr<dtk::Context>& context)
        {
            auto out = std::shared_ptr<DevicesModel>(new DevicesModel);
            out->_init(context);
            return out;
        }

        std::shared_ptr<dtk::IObservableValue<DevicesModelData> > DevicesModel::observeData() const
        {
            return _p->data;
        }

        void DevicesModel::setDeviceIndex(int index)
        {
            DTK_P();
            if (index == p.deviceIndex)
                return;
            p.deviceIndex = index;
            _update();
        }

        void DevicesModel::setDisplayModeIndex(int index)
        {
            DTK_P();
            if (index == p.displayModeIndex)
                return;
            p.displayModeIndex = index;
            _update();
        }

        void DevicesModel::setPixelTypeIndex(int index)
        {
            DTK_P();
            if (index == p.pixelTypeIndex)
                return;
            p.pixelTypeIndex = index;
            _update();
        }

        void DevicesModel::setDeviceEnabled(bool value)
        {
            DTK_P();
            if (value == p.deviceEnabled)
                return;
            p.deviceEnabled = value;
            _update();
        }

        void DevicesModel::setBoolOptions(const BoolOptions& value)
        {
            DTK_P();
            if (value == p.boolOptions)
                return;
            p.boolOptions = value;
            _update();
        }

        void DevicesModel::setVideoLevels(dtk::VideoLevels value)
        {
            DTK_P();
            if (value == p.videoLevels)
                return;
            p.videoLevels = value;
            _update();
        }

        void DevicesModel::setHDRMode(HDRMode value)
        {
            DTK_P();
            if (value == p.hdrMode)
                return;
            p.hdrMode = value;
            _update();
        }

        void DevicesModel::setHDRData(const image::HDRData& value)
        {
            DTK_P();
            if (value == p.hdrData)
                return;
            p.hdrData = value;
            _update();
        }

        void DevicesModel::_update()
        {
            DTK_P();

            DevicesModelData data;

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

            data.pixelTypes.push_back(PixelType::None);
            if (!p.deviceInfo.empty() && p.deviceIndex >= 1 && (p.deviceIndex - 1) < p.deviceInfo.size())
            {
                for (const auto& i : p.deviceInfo[p.deviceIndex - 1].pixelTypes)
                {
                    data.pixelTypes.push_back(i);
                }
                data.pixelTypeIndex = p.pixelTypeIndex;
            }

            data.deviceEnabled = p.deviceEnabled;

            data.boolOptions = p.boolOptions;

            data.videoLevels = p.videoLevels;

            data.hdrMode = p.hdrMode;
            data.hdrData = p.hdrData;

            p.data->setIfChanged(data);
        }
    }
}
