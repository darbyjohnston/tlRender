// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/IDeviceSystem.h>

namespace tl
{
    namespace device
    {
        void IDeviceSystem::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init(name, context);

            _context = context;

            _deviceInfo = observer::List<DeviceInfo>::create();
        }

        IDeviceSystem::IDeviceSystem()
        {}

        IDeviceSystem::~IDeviceSystem()
        {}

        std::shared_ptr<observer::IList<DeviceInfo> > IDeviceSystem::observeDeviceInfo() const
        {
            return _deviceInfo;
        }

        std::chrono::milliseconds IDeviceSystem::getTickTime() const
        {
            return std::chrono::milliseconds(1000);
        }
    }
}
