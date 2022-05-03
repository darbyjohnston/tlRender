// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>
#include <tlCore/Image.h>
#include <tlCore/ListObserver.h>
#include <tlCore/Time.h>

#include "DeckLinkAPI.h"

namespace tl
{
    namespace bmd
    {
        //! Display mode.
        struct DisplayMode
        {
            BMDDisplayMode displayMode = BMDDisplayMode::bmdModeUnknown;
            imaging::Size size;
            otime::RationalTime frameRate;

            bool operator == (const DisplayMode&) const;
        };

        //! Device information.
        struct DeviceInfo
        {
            std::string model;
            std::vector<DisplayMode> displayModes;

            bool operator == (const DeviceInfo&) const;
        };

        //! Device system.
        class DeviceSystem : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(DeviceSystem);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            DeviceSystem();

        public:
            ~DeviceSystem() override;

            //! Create a new device system.
            static std::shared_ptr<DeviceSystem> create(const std::shared_ptr<system::Context>&);

            //! Observe the device information.
            std::shared_ptr<observer::IList<DeviceInfo> > observeDeviceInfo() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
