// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>
#include <tlCore/ISystem.h>

#include <dtk/core/ObservableList.h>
#include <dtk/core/ObservableValue.h>

namespace tl
{
    namespace audio
    {
        //! Audio device ID.
        struct DeviceID
        {
            int         number = -1;
            std::string name;

            bool operator == (const DeviceID&) const;
            bool operator != (const DeviceID&) const;
        };

        //! Audio device information.
        struct DeviceInfo
        {
            DeviceID    id;
            audio::Info info;

            bool operator == (const DeviceInfo&) const;
            bool operator != (const DeviceInfo&) const;
        };

        //! Audio system.
        class System : public system::ISystem
        {
            DTK_NON_COPYABLE(System);

        protected:
            System(const std::shared_ptr<dtk::Context>&);

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<dtk::Context>&);

            //! Get the list of audio drivers.
            const std::vector<std::string>& getDrivers() const;

            //! Get the list of audio devices.
            const std::vector<DeviceInfo>& getDevices() const;

            //! Observe the list of audio devices.
            std::shared_ptr<dtk::IObservableList<DeviceInfo> > observeDevices() const;

            //! Get the default audio device.
            DeviceInfo getDefaultDevice() const;

            //! Observe the default audio device.
            std::shared_ptr<dtk::IObservableValue<DeviceInfo> > observeDefaultDevice() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            std::vector<DeviceInfo> _getDevices();
            DeviceInfo _getDefaultDevice();

            void _run();

            DTK_PRIVATE();
        };
    }
}
