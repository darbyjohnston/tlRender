// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Audio.h>
#include <tlrCore/ISystem.h>

namespace tlr
{
    namespace audio
    {
        //! Audio device format.
        enum class DeviceFormat
        {
            S8,
            S16,
            S24,
            S32,
            F32,
            F64,

            Count,
            First = S8
        };
        TLR_ENUM(DeviceFormat);
        TLR_ENUM_SERIALIZE(DeviceFormat);

        //! Audio device.
        struct Device
        {
            std::string               name;
            size_t                    outputChannels      = 0;
            size_t                    inputChannels       = 0;
            size_t                    duplexChannels      = 0;
            std::vector<size_t>       sampleRates;
            size_t                    preferredSampleRate = 0;
            std::vector<DeviceFormat> nativeFormats;
        };

        //! Audio system.
        class System : public core::ISystem
        {
            TLR_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<core::Context>&);
            System();

        public:
            ~System() override;

            static std::shared_ptr<System> create(const std::shared_ptr<core::Context>&);

            const std::vector<std::string>& getAPIs() const;
            const std::vector<Device>& getDevices() const;

            size_t getDefaultInputDevice() const;
            size_t getDefaultOutputDevice() const;

            Info getDefaultOutputInfo() const;
            Info getDefaultInputInfo() const;

        private:
            TLR_PRIVATE();
        };
    }
}
