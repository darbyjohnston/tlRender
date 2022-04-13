// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/ListObserver.h>

namespace tl
{
    namespace bmd
    {
        //! Device information.
        struct DeviceInfo
        {
            std::string model;

            bool operator == (const DeviceInfo&) const;
        };

        //! Device information system.
        class DeviceInfoSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(DeviceInfoSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            DeviceInfoSystem();

        public:
            ~DeviceInfoSystem() override;

            //! Create a new device information system.
            static std::shared_ptr<DeviceInfoSystem> create(const std::shared_ptr<system::Context>&);

            //! Observe device information.
            std::shared_ptr<observer::IList<DeviceInfo> > observeDeviceInfo() const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
