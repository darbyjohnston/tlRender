// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlDevice/BMDDevicesModel.h>

#include <tlDevice/BMDSystem.h>

#include <ftk/Core/Context.h>

#include <sstream>

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
            ftk::VideoLevels videoLevels = ftk::VideoLevels::LegalRange;
            HDRMode hdrMode = HDRMode::FromFile;
            image::HDRData hdrData;
            std::shared_ptr<ftk::ObservableValue<DevicesModelData> > data;
            std::shared_ptr<ftk::ListObserver<DeviceInfo> > deviceInfoObserver;
        };

        void DevicesModel::_init(
            const std::shared_ptr<ftk::Context>& context)
        {
            FTK_P();

            p.data = ftk::ObservableValue<DevicesModelData>::create();

            _update();

            if (auto system = context->getSystem<System>())
            {
                p.deviceInfoObserver = ftk::ListObserver<DeviceInfo>::create(
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
            const std::shared_ptr<ftk::Context>& context)
        {
            auto out = std::shared_ptr<DevicesModel>(new DevicesModel);
            out->_init(context);
            return out;
        }

        std::shared_ptr<ftk::IObservableValue<DevicesModelData> > DevicesModel::observeData() const
        {
            return _p->data;
        }

        void DevicesModel::setDeviceIndex(int index)
        {
            FTK_P();
            if (index == p.deviceIndex)
                return;
            p.deviceIndex = index;
            _update();
        }

        void DevicesModel::setDisplayModeIndex(int index)
        {
            FTK_P();
            if (index == p.displayModeIndex)
                return;
            p.displayModeIndex = index;
            _update();
        }

        void DevicesModel::setPixelTypeIndex(int index)
        {
            FTK_P();
            if (index == p.pixelTypeIndex)
                return;
            p.pixelTypeIndex = index;
            _update();
        }

        void DevicesModel::setDeviceEnabled(bool value)
        {
            FTK_P();
            if (value == p.deviceEnabled)
                return;
            p.deviceEnabled = value;
            _update();
        }

        void DevicesModel::setBoolOptions(const BoolOptions& value)
        {
            FTK_P();
            if (value == p.boolOptions)
                return;
            p.boolOptions = value;
            _update();
        }

        void DevicesModel::setVideoLevels(ftk::VideoLevels value)
        {
            FTK_P();
            if (value == p.videoLevels)
                return;
            p.videoLevels = value;
            _update();
        }

        void DevicesModel::setHDRMode(HDRMode value)
        {
            FTK_P();
            if (value == p.hdrMode)
                return;
            p.hdrMode = value;
            _update();
        }

        void DevicesModel::setHDRData(const image::HDRData& value)
        {
            FTK_P();
            if (value == p.hdrData)
                return;
            p.hdrData = value;
            _update();
        }

        void DevicesModel::_update()
        {
            FTK_P();

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

        void to_json(nlohmann::json& json, const DevicesModelData& value)
        {
            json["DeviceIndex"] = value.deviceIndex;
            json["DisplayModeIndex"] = value.displayModeIndex;
            json["PixelTypeIndex"] = value.pixelTypeIndex;
            json["DeviceEnabled"] = value.deviceEnabled;
            nlohmann::json json2;
            for (const auto& i : value.boolOptions)
            {
                std::stringstream ss;
                ss << i.first;
                json2[ss.str()] = i.second;
            }
            json["BoolOptions"] = json2;
            json["HDRMode"] = to_string(value.hdrMode);
            json["HDRData"] = value.hdrData;
        }

        void from_json(const nlohmann::json& json, DevicesModelData& value)
        {
            json.at("DeviceIndex").get_to(value.deviceIndex);
            json.at("DisplayModeIndex").get_to(value.displayModeIndex);
            json.at("PixelTypeIndex").get_to(value.pixelTypeIndex);
            json.at("DeviceEnabled").get_to(value.deviceEnabled);
            for (const auto& i : getOptionEnums())
            {
                std::stringstream ss;
                ss << i;
                if (json.at("BoolOptions").contains(ss.str()))
                {
                    value.boolOptions[i] = json.at("BoolOptions").at(ss.str()).get<bool>();
                }
            }
            from_string(json.at("HDRMode").get<std::string>(), value.hdrMode);
            json.at("HDRData").get_to(value.hdrData);
        }
    }
}
