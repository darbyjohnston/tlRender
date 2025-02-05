// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/AudioModel.h>

#include <tlPlay/Settings.h>

#include <tlCore/AudioSystem.h>

#include <dtk/core/Context.h>
#include <dtk/core/Math.h>

namespace tl
{
    namespace play
    {
        struct AudioModel::Private
        {
            std::shared_ptr<Settings> settings;
            std::shared_ptr<dtk::ObservableList<audio::DeviceID> > devices;
            std::shared_ptr<dtk::ObservableValue<audio::DeviceID> > device;
            std::shared_ptr<dtk::ObservableValue<float> > volume;
            std::shared_ptr<dtk::ObservableValue<bool> > mute;
            std::shared_ptr<dtk::ObservableList<bool> > channelMute;
            std::shared_ptr<dtk::ObservableValue<double> > syncOffset;
            std::shared_ptr<dtk::ListObserver<audio::DeviceInfo> > devicesObserver;
        };

        void AudioModel::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<Settings>& settings)
        {
            DTK_P();

            p.settings = settings;

            p.devices = dtk::ObservableList<audio::DeviceID>::create();
            p.device = dtk::ObservableValue<audio::DeviceID>::create();

            p.settings->setDefaultValue("Audio/Volume", 1.F);
            p.volume = dtk::ObservableValue<float>::create(
                p.settings->getValue<float>("Audio/Volume"));

            p.settings->setDefaultValue("Audio/Mute", false);
            p.mute = dtk::ObservableValue<bool>::create(
                p.settings->getValue<bool>("Audio/Mute"));

            p.channelMute = dtk::ObservableList<bool>::create();

            p.syncOffset = dtk::ObservableValue<double>::create(0.0);

            auto audioSystem = context->getSystem<audio::System>();
            p.devicesObserver = dtk::ListObserver<audio::DeviceInfo>::create(
                audioSystem->observeDevices(),
                [this](const std::vector<audio::DeviceInfo>& devices)
                {
                    std::vector<audio::DeviceID> ids;
                    for (const auto& device : devices)
                    {
                        ids.push_back(device.id);
                    }
                    _p->devices->setIfChanged(ids);
                });
        }

        AudioModel::AudioModel() :
            _p(new Private)
        {}

        AudioModel::~AudioModel()
        {}

        std::shared_ptr<AudioModel> AudioModel::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<Settings>& settings)
        {
            auto out = std::shared_ptr<AudioModel>(new AudioModel);
            out->_init(context, settings);
            return out;
        }

        const std::vector<audio::DeviceID>& AudioModel::getDevices()
        {
            return _p->devices->get();
        }

        std::shared_ptr<dtk::IObservableList<audio::DeviceID> > AudioModel::observeDevices() const
        {
            return _p->devices;
        }

        const audio::DeviceID& AudioModel::getDevice() const
        {
            return _p->device->get();
        }

        std::shared_ptr<dtk::IObservableValue<audio::DeviceID> > AudioModel::observeDevice() const
        {
            return _p->device;
        }

        void AudioModel::setDevice(const audio::DeviceID& value)
        {
            _p->device->setIfChanged(value);
        }

        float AudioModel::getVolume() const
        {
            return _p->volume->get();
        }

        std::shared_ptr<dtk::IObservableValue<float> > AudioModel::observeVolume() const
        {
            return _p->volume;
        }

        void AudioModel::setVolume(float value)
        {
            const float tmp = dtk::clamp(value, 0.F, 1.F);
            _p->settings->setValue("Audio/Volume", tmp);
            _p->volume->setIfChanged(tmp);
        }

        void AudioModel::volumeUp()
        {
            setVolume(_p->volume->get() + .1F);
        }

        void AudioModel::volumeDown()
        {
            setVolume(_p->volume->get() - .1F);
        }

        bool AudioModel::isMuted() const
        {
            return _p->mute->get();
        }

        std::shared_ptr<dtk::IObservableValue<bool> > AudioModel::observeMute() const
        {
            return _p->mute;
        }

        void AudioModel::setMute(bool value)
        {
            _p->settings->setValue("Audio/Mute", value);
            _p->mute->setIfChanged(value);
        }

        const std::vector<bool>& AudioModel::getChannelMute() const
        {
            return _p->channelMute->get();
        }

        std::shared_ptr<dtk::IObservableList<bool> > AudioModel::observeChannelMute() const
        {
            return _p->channelMute;
        }

        void AudioModel::setChannelMute(const std::vector<bool>& value)
        {
            _p->channelMute->setIfChanged(value);
        }

        double AudioModel::getSyncOffset() const
        {
            return _p->syncOffset->get();
        }

        std::shared_ptr<dtk::IObservableValue<double> > AudioModel::observeSyncOffset() const
        {
            return _p->syncOffset;
        }

        void AudioModel::setSyncOffset(double value)
        {
            _p->syncOffset->setIfChanged(value);
        }
    }
}
