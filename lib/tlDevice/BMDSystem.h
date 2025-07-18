// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/BMDData.h>

#include <tlCore/ISystem.h>

#include <feather-tk/core/ObservableList.h>

namespace tl
{
    namespace bmd
    {
        //! BMD system.
        class System : public system::ISystem
        {
            FEATHER_TK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<feather_tk::Context>&);

        public:
            ~System() override;

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<feather_tk::Context>&);

            //! Observe the device information.
            std::shared_ptr<feather_tk::IObservableList<DeviceInfo> > observeDeviceInfo() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            FEATHER_TK_PRIVATE();
        };
    }
}
