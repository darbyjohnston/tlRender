// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDDeviceData.h>

#include <tlCore/ISystem.h>
#include <tlCore/ListObserver.h>

namespace tl
{
    namespace device
    {
        //! BMD device system.
        class BMDDeviceSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(BMDDeviceSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            BMDDeviceSystem();

        public:
            ~BMDDeviceSystem() override;

            //! Create a new BMD device system.
            static std::shared_ptr<BMDDeviceSystem> create(const std::shared_ptr<system::Context>&);

            //! Observe the device information.
            std::shared_ptr<observer::IList<DeviceInfo> > observeDeviceInfo() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
