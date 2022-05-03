// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ValueObserver.h>

#include <memory>
#include <string>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! Device model data.
        struct DeviceModelData
        {
            std::vector<std::string> devices;
            int deviceIndex = 0;
            std::vector<std::string> displayModes;
            int displayModeIndex = 0;

            bool operator == (const DeviceModelData&) const;
        };

        //! Device model.
        class DeviceModel : public std::enable_shared_from_this<DeviceModel>
        {
            TLRENDER_NON_COPYABLE(DeviceModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            DeviceModel();

        public:
            ~DeviceModel();

            //! Create a new device model.
            static std::shared_ptr<DeviceModel> create(const std::shared_ptr<system::Context>&);

            //! Observe the model data.
            std::shared_ptr<observer::IValue<DeviceModelData> > observeData() const;

            //! Set the device.
            void setDevice(int);

            //! Set the display mode.
            void setDisplayMode(int);

        private:
            void _deviceInfoUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
