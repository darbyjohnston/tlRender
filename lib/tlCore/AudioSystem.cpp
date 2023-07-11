// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/AudioSystem.h>

#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>

#if defined(TLRENDER_AUDIO)
#include <rtaudio/RtAudio.h>
#endif // TLRENDER_AUDIO

#include <array>
#include <map>

namespace tl
{
    namespace audio
    {
        TLRENDER_ENUM_IMPL(
            DeviceFormat,
            "S8",
            "S16",
            "S24",
            "S32",
            "F32",
            "F64");
        TLRENDER_ENUM_SERIALIZE_IMPL(DeviceFormat);

        struct System::Private
        {
#if defined(TLRENDER_AUDIO)
            std::unique_ptr<RtAudio> rtAudio;
#endif // TLRENDER_AUDIO
            std::vector<std::string> apis;
            std::vector<Device> devices;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::audio::System", context);
            TLRENDER_P();

#if defined(TLRENDER_AUDIO)
            {
                std::stringstream ss;
                ss << "RtAudio version: " << RtAudio::getVersion();
                _log(ss.str());
            }

            std::vector<RtAudio::Api> rtAudioApis;
            RtAudio::getCompiledApi(rtAudioApis);
            for (auto i : rtAudioApis)
            {
                p.apis.push_back(RtAudio::getApiDisplayName(i));

                std::stringstream ss;
                ss << "Audio API: " << RtAudio::getApiDisplayName(i);
                _log(ss.str());
            }

            try
            {
                p.rtAudio.reset(new RtAudio);
                std::vector<std::string> log;
                log.push_back(std::string());
                const size_t rtDeviceCount = p.rtAudio->getDeviceCount();
                for (size_t i = 0; i < rtDeviceCount; ++i)
                {
                    const RtAudio::DeviceInfo rtInfo = p.rtAudio->getDeviceInfo(i);
                    if (rtInfo.probed)
                    {
                        Device device;
                        device.name = rtInfo.name;
                        device.outputChannels = rtInfo.outputChannels;
                        device.inputChannels = rtInfo.inputChannels;
                        device.duplexChannels = rtInfo.duplexChannels;
                        for (auto j : rtInfo.sampleRates)
                        {
                            device.sampleRates.push_back(j);
                        }
                        device.preferredSampleRate = rtInfo.preferredSampleRate;
                        if (rtInfo.nativeFormats & RTAUDIO_SINT8)
                        {
                            device.nativeFormats.push_back(DeviceFormat::S8);
                        }
                        if (rtInfo.nativeFormats & RTAUDIO_SINT16)
                        {
                            device.nativeFormats.push_back(DeviceFormat::S16);
                        }
                        if (rtInfo.nativeFormats & RTAUDIO_SINT24)
                        {
                            device.nativeFormats.push_back(DeviceFormat::S24);
                        }
                        if (rtInfo.nativeFormats & RTAUDIO_SINT32)
                        {
                            device.nativeFormats.push_back(DeviceFormat::S32);
                        }
                        if (rtInfo.nativeFormats & RTAUDIO_FLOAT32)
                        {
                            device.nativeFormats.push_back(DeviceFormat::F32);
                        }
                        if (rtInfo.nativeFormats & RTAUDIO_FLOAT64)
                        {
                            device.nativeFormats.push_back(DeviceFormat::F64);
                        }
                        p.devices.push_back(device);
                        {
                            std::stringstream ss;
                            ss << "    Device " << i << ": " << device.name;
                            log.push_back(ss.str());
                        }
                        {
                            std::stringstream ss;
                            ss << "        Channels: " <<
                                device.outputChannels << " output, " <<
                                device.inputChannels << " input, " <<
                                device.duplexChannels << " duplex";
                            log.push_back(ss.str());
                        }
                        {
                            std::stringstream ss;
                            ss << "        Sample rates: ";
                            for (auto j : device.sampleRates)
                            {
                                ss << j << " ";
                            }
                            log.push_back(ss.str());
                        }
                        {
                            std::stringstream ss;
                            ss << "        Preferred sample rate: " << device.preferredSampleRate;
                            log.push_back(ss.str());
                        }
                        {
                            std::stringstream ss;
                            ss << "        Native formats: ";
                            for (auto j : device.nativeFormats)
                            {
                                ss << j << " ";
                            }
                            log.push_back(ss.str());
                        }
                    }
                }
                {
                    std::stringstream ss;
                    ss << "    Default input device: " << getDefaultInputDevice();
                    log.push_back(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "    Default input info: " <<
                        static_cast<size_t>(getDefaultInputInfo().channelCount) << " " <<
                        getDefaultInputInfo().dataType << " " <<
                        getDefaultInputInfo().sampleRate;
                    log.push_back(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "    Default output device: " << getDefaultOutputDevice();
                    log.push_back(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "    Default output info: " <<
                        static_cast<size_t>(getDefaultOutputInfo().channelCount) << " " <<
                        getDefaultOutputInfo().dataType << " " <<
                        getDefaultOutputInfo().sampleRate;
                    log.push_back(ss.str());
                }
                _log(string::join(log, "\n"));
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot initalize audio system: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO
        }

        System::System() :
            _p(new Private)
        {}

        System::~System()
        {}

        std::shared_ptr<System> System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System);
                out->_init(context);
            }
            return out;
        }

        const std::vector<std::string>& System::getAPIs() const
        {
            return _p->apis;
        }

        const std::vector<Device>& System::getDevices() const
        {
            return _p->devices;
        }

        size_t System::getDefaultInputDevice() const
        {
            TLRENDER_P();
            size_t out = 0;
#if defined(TLRENDER_AUDIO)
            out = p.rtAudio->getDefaultInputDevice();
            const size_t rtDeviceCount = p.rtAudio->getDeviceCount();
            std::vector<size_t> inputChannels;
            for (size_t i = 0; i < rtDeviceCount; ++i)
            {
                const RtAudio::DeviceInfo rtInfo = p.rtAudio->getDeviceInfo(i);
                inputChannels.push_back(rtInfo.inputChannels);
            }
            if (out < inputChannels.size())
            {
                if (0 == inputChannels[out])
                {
                    for (out = 0; out < rtDeviceCount; ++out)
                    {
                        if (inputChannels[out] > 0)
                        {
                            break;
                        }
                    }
                }
            }
#endif // TLRENDER_AUDIO
            return out;
        }

        size_t System::getDefaultOutputDevice() const
        {
            TLRENDER_P();
            size_t out = 0;
#if defined(TLRENDER_AUDIO)
            out = p.rtAudio->getDefaultOutputDevice();
            const size_t rtDeviceCount = p.rtAudio->getDeviceCount();
            std::vector<size_t> outputChannels;
            for (size_t i = 0; i < rtDeviceCount; ++i)
            {
                const RtAudio::DeviceInfo rtInfo = p.rtAudio->getDeviceInfo(i);
                outputChannels.push_back(rtInfo.outputChannels);
            }
            if (out < outputChannels.size())
            {
                if (0 == outputChannels[out])
                {
                    for (out = 0; out < rtDeviceCount; ++out)
                    {
                        if (outputChannels[out] > 0)
                        {
                            break;
                        }
                    }
                }
            }
#endif // TLRENDER_AUDIO
            return out;
        }

        namespace
        {
            DeviceFormat getBestFormat(std::vector<DeviceFormat> value)
            {
                std::sort(
                    value.begin(),
                    value.end(),
                    [](DeviceFormat a, DeviceFormat b)
                    {
                        return static_cast<size_t>(a) < static_cast<size_t>(b);
                    });
                return !value.empty() ? value.back() : DeviceFormat::F32;
            }
        }

        Info System::getDefaultInputInfo() const
        {
            TLRENDER_P();
            Info out;
            const size_t deviceIndex = getDefaultInputDevice();
            if (deviceIndex < p.devices.size())
            {
                const auto& device = p.devices[deviceIndex];
                out.channelCount = device.inputChannels;
                switch (getBestFormat(device.nativeFormats))
                {
                case DeviceFormat::S8: out.dataType = DataType::S8; break;
                case DeviceFormat::S16: out.dataType = DataType::S16; break;
                case DeviceFormat::S24:
                case DeviceFormat::S32: out.dataType = DataType::S32; break;
                case DeviceFormat::F32: out.dataType = DataType::F32; break;
                case DeviceFormat::F64: out.dataType = DataType::F64; break;
                default: out.dataType = DataType::F32; break;
                }
                out.sampleRate = device.preferredSampleRate;
            }
            return out;
        }

        Info System::getDefaultOutputInfo() const
        {
            TLRENDER_P();
            Info out;
            const size_t deviceIndex = getDefaultOutputDevice();
            if (deviceIndex < p.devices.size())
            {
                const auto& device = p.devices[deviceIndex];
                out.channelCount = device.outputChannels;
                switch (getBestFormat(device.nativeFormats))
                {
                case DeviceFormat::S8: out.dataType = DataType::S8; break;
                case DeviceFormat::S16: out.dataType = DataType::S16; break;
                case DeviceFormat::S24:
                case DeviceFormat::S32: out.dataType = DataType::S32; break;
                case DeviceFormat::F32: out.dataType = DataType::F32; break;
                case DeviceFormat::F64: out.dataType = DataType::F64; break;
                default: out.dataType = DataType::F32; break;
                }
                out.sampleRate = device.preferredSampleRate;
            }
            return out;
        }
    }
}
