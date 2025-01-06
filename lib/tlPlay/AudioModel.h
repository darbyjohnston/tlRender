// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/CompareOptions.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/ListObserver.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        class Settings;

        //! Audio model.
        class AudioModel : public std::enable_shared_from_this<AudioModel>
        {
            TLRENDER_NON_COPYABLE(AudioModel);

        protected:
            void _init(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<system::Context>&);

            AudioModel();

        public:
            ~AudioModel();

            //! Create a new model.
            static std::shared_ptr<AudioModel> create(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<system::Context>&);

            //! Get the output devices.
            const std::vector<audio::DeviceID>& getDevices();

            //! Observe the output devices.
            std::shared_ptr<observer::IList<audio::DeviceID> > observeDevices() const;

            //! Get the output device.
            const audio::DeviceID& getDevice() const;

            //! Observe the output device.
            std::shared_ptr<observer::IValue<audio::DeviceID> > observeDevice() const;

            //! Set the output device.
            void setDevice(const audio::DeviceID&);

            //! Get the volume.
            float getVolume() const;

            //! Observe the volume.
            std::shared_ptr<observer::IValue<float> > observeVolume() const;

            //! Set the volume.
            void setVolume(float);

            //! Increase the volume.
            void volumeUp();

            //! Decrease the volume.
            void volumeDown();

            //! Get the audio mute.
            bool isMuted() const;

            //! Observe the audio mute.
            std::shared_ptr<observer::IValue<bool> > observeMute() const;

            //! Set the audio mute.
            void setMute(bool);

            //! Get the audio channels mute.
            const std::vector<bool>& getChannelMute() const;

            //! Observe the audio channels mute.
            std::shared_ptr<observer::IList<bool> > observeChannelMute() const;

            //! Set the audio channels mute.
            void setChannelMute(const std::vector<bool>&);

            //! Get the audio sync offset.
            double getSyncOffset() const;

            //! Set the audio sync offset.
            std::shared_ptr<observer::IValue<double> > observeSyncOffset() const;

            //! Set the audio sync offset.
            void setSyncOffset(double);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
