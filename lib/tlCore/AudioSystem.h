// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>
#include <tlCore/ISystem.h>

namespace tl
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
        TLRENDER_ENUM(DeviceFormat);
        TLRENDER_ENUM_SERIALIZE(DeviceFormat);

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
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            System();

        public:
            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System> create(const std::shared_ptr<system::Context>&);

            //! Get the list of audio APIs.
            const std::vector<std::string>& getAPIs() const;

            //! Get the audio devices.
            const std::vector<Device>& getDevices() const;

            //! Get the default audio input device.
            size_t getDefaultInputDevice() const;

            //! Get the default audio output device.
            size_t getDefaultOutputDevice() const;

            //! Get the default audio input device information.
            Info getDefaultInputInfo() const;

            //! Get the default audio output device information.
            Info getDefaultOutputInfo() const;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
