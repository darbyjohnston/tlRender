// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/IDeviceSystem.h>

namespace tl
{
    namespace device
    {
        //! BMD device system.
        class BMDDeviceSystem : public IDeviceSystem
        {
            TLRENDER_NON_COPYABLE(BMDDeviceSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            BMDDeviceSystem();

        public:
            ~BMDDeviceSystem() override;

            //! Create a new BMD device system.
            static std::shared_ptr<BMDDeviceSystem> create(const std::shared_ptr<system::Context>&);

            virtual std::shared_ptr<IOutputDevice> createDevice(int deviceIndex, int displayModeIndex) override;
            void tick() override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
