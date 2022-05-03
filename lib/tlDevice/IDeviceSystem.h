// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/Image.h>
#include <tlCore/ListObserver.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace device
    {
        class IOutputDevice;

        //! Display mode.
        struct DisplayMode
        {
            std::string name;
            imaging::Size size;
            otime::RationalTime frameRate;

            bool operator == (const DisplayMode&) const;
        };

        //! Device information.
        struct DeviceInfo
        {
            std::string name;
            std::vector<DisplayMode> displayModes;

            bool operator == (const DeviceInfo&) const;
        };

        //! Base class for device systems.
        class IDeviceSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(IDeviceSystem);

        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<system::Context>&);

            IDeviceSystem();

        public:
            ~IDeviceSystem() override = 0;

            //! Observe the device information.
            std::shared_ptr<observer::IList<DeviceInfo> > observeDeviceInfo() const;

            //! Create a new output device.
            virtual std::shared_ptr<IOutputDevice> createDevice(int deviceIndex, int displayModeIndex) = 0;

            std::chrono::milliseconds getTickTime() const override;

        protected:
            std::weak_ptr<system::Context> _context;
            std::shared_ptr<observer::List<DeviceInfo> > _deviceInfo;
        };
    }
}
