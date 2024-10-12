// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/AudioSystem.h>

#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#if defined(TLRENDER_AUDIO)
#include <rtaudio/RtAudio.h>
#endif // TLRENDER_AUDIO

#include <array>
#include <atomic>
#include <map>
#include <mutex>
#include <thread>

namespace tl
{
    namespace audio
    {
        bool DeviceID::operator == (const DeviceID& other) const
        {
            return
                number == other.number &&
                name == other.name;
        }

        bool DeviceID::operator != (const DeviceID& other) const
        {
            return !(*this == other);
        }

        TLRENDER_ENUM_IMPL(
            DeviceFormat,
            "S8",
            "S16",
            "S24",
            "S32",
            "F32",
            "F64");
        TLRENDER_ENUM_SERIALIZE_IMPL(DeviceFormat);

        bool DeviceInfo::operator == (const DeviceInfo& other) const
        {
            return
                id == other.id &&
                outputChannels == other.outputChannels &&
                inputChannels == other.inputChannels &&
                duplexChannels == other.duplexChannels &&
                sampleRates == other.sampleRates &&
                preferredSampleRate == other.preferredSampleRate &&
                nativeFormats == other.nativeFormats &&
                outputInfo == other.outputInfo &&
                inputInfo == other.inputInfo;
        }

        bool DeviceInfo::operator != (const DeviceInfo& other) const
        {
            return !(*this == other);
        }

        struct System::Private
        {
#if defined(TLRENDER_AUDIO)
            std::unique_ptr<RtAudio> rtAudio;
#endif // TLRENDER_AUDIO
            std::vector<std::string> apis;
            std::shared_ptr<observer::List<DeviceInfo> > devices;
            std::shared_ptr<observer::Value<DeviceID> > defaultOutputDevice;
            std::shared_ptr<observer::Value<DeviceID> > defaultInputDevice;

            struct Mutex
            {
                std::vector<DeviceInfo> devices;
                DeviceID defaultOutputDevice;
                DeviceID defaultInputDevice;
                std::mutex mutex;
            };
            Mutex mutex;
            struct Thread
            {
                std::vector<DeviceInfo> devices;
                DeviceID defaultOutputDevice;
                DeviceID defaultInputDevice;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::audio::System", context);
            TLRENDER_P();

#if defined(TLRENDER_AUDIO)
            try
            {
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

                RtAudio::Api rtApi = RtAudio::Api::UNSPECIFIED;
#if defined(__linux__)
                rtApi = RtAudio::Api::LINUX_ALSA;
#endif // __linux__
                p.rtAudio.reset(new RtAudio(rtApi));
                p.rtAudio->showWarnings(false);
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot initialize audio system: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO

            const std::vector<DeviceInfo> devices = _getDevices();
            const DeviceID defaultOutputDevice = _getDefaultOutputDevice(devices);
            const DeviceID defaultInputDevice = _getDefaultInputDevice(devices);

            p.devices = observer::List<DeviceInfo>::create(devices);
            p.defaultOutputDevice = observer::Value<DeviceID>::create(defaultOutputDevice);
            p.defaultInputDevice = observer::Value<DeviceID>::create(defaultInputDevice);

            p.mutex.devices = devices;
            p.mutex.defaultOutputDevice = defaultOutputDevice;
            p.mutex.defaultInputDevice = defaultInputDevice;

#if defined(TLRENDER_AUDIO)
            if (p.rtAudio)
            {
                p.thread.running = true;
                p.thread.thread = std::thread(
                    [this]
                    {
                        while (_p->thread.running)
                        {
                            _run();
                        }
                    });
            }
#endif // TLRENDER_AUDIO
        }

        System::System() :
            _p(new Private)
        {}

        System::~System()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

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

        const std::vector<DeviceInfo>& System::getDevices() const
        {
            return _p->devices->get();
        }

        std::shared_ptr<observer::IList<DeviceInfo> > System::observeDevices() const
        {
            return _p->devices;
        }

        DeviceID System::getDefaultOutputDevice() const
        {
            return _p->defaultOutputDevice->get();
        }

        std::shared_ptr<observer::IValue<DeviceID> > System::observeDefaultOutputDevice() const
        {
            return _p->defaultOutputDevice;
        }

        DeviceID System::getDefaultInputDevice() const
        {
            return _p->defaultInputDevice->get();
        }

        std::shared_ptr<observer::IValue<DeviceID> > System::observeDefaultInputDevice() const
        {
            return _p->defaultInputDevice;
        }

        void System::tick()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> devices;
            DeviceID defaultOutputDevice;
            DeviceID defaultInputDevice;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                devices = p.mutex.devices;
                defaultOutputDevice = p.mutex.defaultOutputDevice;
                defaultInputDevice = p.mutex.defaultInputDevice;
            }
            p.devices->setIfChanged(devices);
            p.defaultOutputDevice->setIfChanged(defaultOutputDevice);
            p.defaultInputDevice->setIfChanged(defaultInputDevice);
        }

        std::chrono::milliseconds System::getTickTime() const
        {
            return std::chrono::milliseconds(500);
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

        std::vector<DeviceInfo> System::_getDevices()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> out;
#if defined(TLRENDER_AUDIO)
            try
            {
                std::vector<RtAudio::DeviceInfo> rtInfoList;
                for (unsigned int i = 0; i < p.rtAudio->getDeviceCount(); ++i)
                {
                    rtInfoList.push_back(p.rtAudio->getDeviceInfo(i));
                }

                for (size_t i = 0; i < rtInfoList.size(); ++i)
                {
                    const auto& rtInfo = rtInfoList[i];
                    if (rtInfo.probed)
                    {
                        DeviceInfo device;
                        device.id.number = i;
                        device.id.name = rtInfo.name;
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

                        device.outputInfo.channelCount = device.outputChannels;
                        switch (getBestFormat(device.nativeFormats))
                        {
                        case DeviceFormat::S8: device.outputInfo.dataType = DataType::S8; break;
                        case DeviceFormat::S16: device.outputInfo.dataType = DataType::S16; break;
                        case DeviceFormat::S24:
                        case DeviceFormat::S32: device.outputInfo.dataType = DataType::S32; break;
                        case DeviceFormat::F32: device.outputInfo.dataType = DataType::F32; break;
                        case DeviceFormat::F64: device.outputInfo.dataType = DataType::F64; break;
                        default: device.outputInfo.dataType = DataType::F32; break;
                        }
                        device.outputInfo.sampleRate = device.preferredSampleRate;

                        device.inputInfo.channelCount = device.inputChannels;
                        switch (getBestFormat(device.nativeFormats))
                        {
                        case DeviceFormat::S8: device.inputInfo.dataType = DataType::S8; break;
                        case DeviceFormat::S16: device.inputInfo.dataType = DataType::S16; break;
                        case DeviceFormat::S24:
                        case DeviceFormat::S32: device.inputInfo.dataType = DataType::S32; break;
                        case DeviceFormat::F32: device.inputInfo.dataType = DataType::F32; break;
                        case DeviceFormat::F64: device.inputInfo.dataType = DataType::F64; break;
                        default: device.inputInfo.dataType = DataType::F32; break;
                        }
                        device.inputInfo.sampleRate = device.preferredSampleRate;

                        out.push_back(device);
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot get audio devices: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO
            return out;
        }

        DeviceID System::_getDefaultOutputDevice(const std::vector<DeviceInfo>& devices)
        {
            TLRENDER_P();
            DeviceID out;
#if defined(TLRENDER_AUDIO)
            try
            {
                unsigned int id = p.rtAudio->getDefaultOutputDevice();
                for (size_t i = 0; i < devices.size(); ++i)
                {
                    if (id == devices[i].id.number)
                    {
                        out = devices[i].id;
                        break;
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot get default audio output device: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO
            return out;
        }

        DeviceID System::_getDefaultInputDevice(const std::vector<DeviceInfo>& devices)
        {
            TLRENDER_P();
            DeviceID out;
#if defined(TLRENDER_AUDIO)
            try
            {
                unsigned int id = p.rtAudio->getDefaultInputDevice();
                for (size_t i = 0; i < devices.size(); ++i)
                {
                    if (id == devices[i].id.number)
                    {
                        out = devices[i].id;
                        break;
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot get default audio input device: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO
            return out;
        }

        void System::_run()
        {
            TLRENDER_P();
#if defined(TLRENDER_AUDIO)

            const std::vector<DeviceInfo> devices = _getDevices();
            const DeviceID defaultOutputDevice = _getDefaultOutputDevice(devices);
            const DeviceID defaultInputDevice = _getDefaultInputDevice(devices);

            if (devices != p.thread.devices)
            {
                p.thread.devices = devices;

                std::vector<std::string> log;
                log.push_back(std::string());
                for (size_t i = 0; i < devices.size(); ++i)
                {
                    const auto& device = devices[i];
                    {
                        std::stringstream ss;
                        ss << "    Device " << i << ": " << device.id.name;
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
                        ss << "        Preferred sample rate: " <<
                            device.preferredSampleRate;
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
                _log(string::join(log, "\n"));
            }
            if (defaultOutputDevice != p.thread.defaultOutputDevice)
            {
                p.thread.defaultOutputDevice = defaultOutputDevice;

                std::stringstream ss;
                ss << "    Default output device: " <<
                    defaultOutputDevice.number << " " <<
                    defaultOutputDevice.name;
                _log(ss.str());
            }
            if (defaultInputDevice != p.thread.defaultInputDevice)
            {
                p.thread.defaultInputDevice = defaultInputDevice;

                std::stringstream ss;
                ss << "    Default input device: " <<
                    defaultInputDevice.number << " " <<
                    defaultInputDevice.name;
                _log(ss.str());
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.devices = p.thread.devices;
                p.mutex.defaultOutputDevice = p.thread.defaultOutputDevice;
                p.mutex.defaultInputDevice = p.thread.defaultInputDevice;
            }
#endif // TLRENDER_AUDIO
        }
    }
}
