// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/DeviceData.h>

#include <tlCore/ISystem.h>
#include <tlCore/ListObserver.h>

namespace tl
{
    namespace device
    {
        class IOutputDevice;

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
            virtual std::shared_ptr<IOutputDevice> createDevice(
                int deviceIndex,
                int displayModeIndex,
                PixelType) = 0;

            std::chrono::milliseconds getTickTime() const override;

        protected:
            std::weak_ptr<system::Context> _context;
            std::shared_ptr<observer::List<DeviceInfo> > _deviceInfo;
        };
    }
}
