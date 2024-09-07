// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>
#include <tlCore/ISystem.h>
#include <tlCore/ListObserver.h>
#include <tlCore/ValueObserver.h>

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

        //! Audio device information.
        struct DeviceInfo
        {
            std::string               name;
            unsigned int              outputChannels      = 0;
            unsigned int              inputChannels       = 0;
            unsigned int              duplexChannels      = 0;
            std::vector<unsigned int> sampleRates;
            unsigned int              preferredSampleRate = 0;
            std::vector<DeviceFormat> nativeFormats;
            Info                      outputInfo;
            Info                      inputInfo;

            bool operator == (const DeviceInfo&) const;
            bool operator != (const DeviceInfo&) const;
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

            //! Get the list of audio devices.
            const std::vector<DeviceInfo>& getDevices() const;

            //! Observe the list of audio devices.
            std::shared_ptr<observer::IList<DeviceInfo> > observeDevices() const;

            //! Get the default audio output device.
            std::string getDefaultOutputDevice() const;

            //! Observe the default audio ouput device.
            std::shared_ptr<observer::IValue<std::string> > observeDefaultOutputDevice() const;

            //! Get the default audio input device.
            std::string getDefaultInputDevice() const;

            //! Observe the default audio input device.
            std::shared_ptr<observer::IValue<std::string> > observeDefaultInputDevice() const;

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            std::vector<DeviceInfo> _getDevices();
            std::string _getDefaultOutputDevice(const std::vector<DeviceInfo>&);
            std::string _getDefaultInputDevice(const std::vector<DeviceInfo>&);

            void _run();

            TLRENDER_PRIVATE();
        };
    }
}
