// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDData.h>

#include <tlCore/ISystem.h>
#include <tlCore/ListObserver.h>

namespace tl
{
    namespace device
    {
        //! BMD system.
        class BMDSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(BMDSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            BMDSystem();

        public:
            ~BMDSystem() override;

            //! Create a new system.
            static std::shared_ptr<BMDSystem> create(const std::shared_ptr<system::Context>&);

            //! Observe the device information.
            std::shared_ptr<observer::IList<DeviceInfo> > observeDeviceInfo() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
