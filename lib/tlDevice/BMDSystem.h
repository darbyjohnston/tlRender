// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlDevice/BMDData.h>

#include <tlCore/ISystem.h>

#include <ftk/Core/ObservableList.h>

namespace tl
{
    namespace bmd
    {
        //! BMD system.
        class System : public system::ISystem
        {
            FTK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<ftk::Context>&);

        public:
            ~System() override;

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<ftk::Context>&);

            //! Observe the device information.
            std::shared_ptr<ftk::IObservableList<DeviceInfo> > observeDeviceInfo() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            FTK_PRIVATE();
        };
    }
}
