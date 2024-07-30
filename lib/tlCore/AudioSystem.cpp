// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
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
#include <atomic>
#include <map>
#include <mutex>
#include <thread>

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

        bool DeviceInfo::operator == (const DeviceInfo& other) const
        {
            return
                name == other.name &&
                outputChannels == other.outputChannels &&
                inputChannels == other.inputChannels &&
                duplexChannels == other.duplexChannels &&
                sampleRates == other.sampleRates &&
                preferredSampleRate == other.preferredSampleRate &&
                nativeFormats == other.nativeFormats;
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
            std::shared_ptr<observer::Value<int> > defaultOutputDevice;
            std::shared_ptr<observer::Value<Info> > defaultOutputInfo;
            std::shared_ptr<observer::Value<int> > defaultInputDevice;
            std::shared_ptr<observer::Value<Info> > defaultInputInfo;

            struct Mutex
            {
                std::vector<DeviceInfo> devices;
                int defaultOutputDevice = -1;
                Info defaultOutputInfo;
                int defaultInputDevice = -1;
                Info defaultInputInfo;
                std::mutex mutex;
            };
            Mutex mutex;
            struct Thread
            {
                std::vector<DeviceInfo> devices;
                int defaultOutputDevice = -1;
                Info defaultOutputInfo;
                int defaultInputDevice = -1;
                Info defaultInputInfo;
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

                p.rtAudio.reset(new RtAudio);
                p.rtAudio->showWarnings(false);
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot initialize audio system: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO

            std::vector<DeviceInfo> devices = _getDevices();
            int defaultOutputDevice = -1;
            Info defaultOutputInfo;
            _getDefaultOutputDevice(devices, defaultOutputDevice, defaultOutputInfo);
            int defaultInputDevice = -1;
            Info defaultInputInfo;
            _getDefaultInputDevice(devices, defaultInputDevice, defaultInputInfo);

            p.devices = observer::List<DeviceInfo>::create(devices);
            p.defaultOutputDevice = observer::Value<int>::create(defaultOutputDevice);
            p.defaultOutputInfo = observer::Value<Info>::create(defaultOutputInfo);
            p.defaultInputDevice = observer::Value<int>::create(defaultInputDevice);
            p.defaultInputInfo = observer::Value<Info>::create(defaultInputInfo);

            p.mutex.devices = devices;
            p.mutex.defaultOutputDevice = defaultOutputDevice;
            p.mutex.defaultOutputInfo = defaultOutputInfo;
            p.mutex.defaultInputDevice = defaultInputDevice;
            p.mutex.defaultInputInfo = defaultInputInfo;

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

        int System::getDefaultOutputDevice() const
        {
            return _p->defaultOutputDevice->get();
        }

        std::shared_ptr<observer::IValue<int> > System::observeDefaultOutputDevice() const
        {
            return _p->defaultOutputDevice;
        }

        Info System::getDefaultOutputInfo() const
        {
            return _p->defaultOutputInfo->get();
        }

        std::shared_ptr<observer::IValue<Info> > System::observeDefaultOutputInfo() const
        {
            return _p->defaultOutputInfo;
        }

        int System::getDefaultInputDevice() const
        {
            return _p->defaultInputDevice->get();
        }

        std::shared_ptr<observer::IValue<int> > System::observeDefaultInputDevice() const
        {
            return _p->defaultInputDevice;
        }

        Info System::getDefaultInputInfo() const
        {
            return _p->defaultInputInfo->get();
        }

        std::shared_ptr<observer::IValue<Info> > System::observeDefaultInputInfo() const
        {
            return _p->defaultInputInfo;
        }

        void System::tick()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> devices;
            int defaultOutputDevice = -1;
            Info defaultOutputInfo;
            int defaultInputDevice = -1;
            Info defaultInputInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                devices = p.mutex.devices;
                defaultOutputDevice = p.mutex.defaultOutputDevice;
                defaultOutputInfo = p.mutex.defaultOutputInfo;
                defaultInputDevice = p.mutex.defaultInputDevice;
                defaultInputInfo = p.mutex.defaultInputInfo;
            }
            p.devices->setIfChanged(devices);
            p.defaultOutputDevice->setIfChanged(defaultOutputDevice);
            p.defaultOutputInfo->setIfChanged(defaultOutputInfo);
            p.defaultInputDevice->setIfChanged(defaultInputDevice);
            p.defaultInputInfo->setIfChanged(defaultInputInfo);
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
                const unsigned int rtDeviceCount = p.rtAudio->getDeviceCount();
                for (unsigned int i = 0; i < rtDeviceCount; ++i)
                {
                    const RtAudio::DeviceInfo rtInfo = p.rtAudio->getDeviceInfo(i);
                    if (rtInfo.probed)
                    {
                        DeviceInfo device;
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

        void System::_getDefaultOutputDevice(const std::vector<DeviceInfo>& devices, int& index, Info& info)
        {
            TLRENDER_P();
            index = -1;
#if defined(TLRENDER_AUDIO)
            try
            {
                unsigned int tmp = p.rtAudio->getDefaultOutputDevice();
                if (tmp < devices.size() && devices[tmp].outputChannels > 0)
                {
                    index = tmp;
                }
                if (index >= 0 && index < devices.size())
                {
                    const auto& device = devices[index];
                    info.channelCount = device.outputChannels;
                    switch (getBestFormat(device.nativeFormats))
                    {
                    case DeviceFormat::S8: info.dataType = DataType::S8; break;
                    case DeviceFormat::S16: info.dataType = DataType::S16; break;
                    case DeviceFormat::S24:
                    case DeviceFormat::S32: info.dataType = DataType::S32; break;
                    case DeviceFormat::F32: info.dataType = DataType::F32; break;
                    case DeviceFormat::F64: info.dataType = DataType::F64; break;
                    default: info.dataType = DataType::F32; break;
                    }
                    info.sampleRate = device.preferredSampleRate;
                }
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot get default audio output device: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO
        }

        void System::_getDefaultInputDevice(const std::vector<DeviceInfo>& devices, int& index, Info& info)
        {
            TLRENDER_P();
            index = -1;
#if defined(TLRENDER_AUDIO)
            try
            {
                unsigned int tmp = p.rtAudio->getDefaultInputDevice();
                if (tmp < devices.size() && devices[tmp].inputChannels > 0)
                {
                    index = tmp;
                }
                if (index >= 0 && index < devices.size())
                {
                    const auto& device = devices[index];
                    info.channelCount = device.inputChannels;
                    switch (getBestFormat(device.nativeFormats))
                    {
                    case DeviceFormat::S8: info.dataType = DataType::S8; break;
                    case DeviceFormat::S16: info.dataType = DataType::S16; break;
                    case DeviceFormat::S24:
                    case DeviceFormat::S32: info.dataType = DataType::S32; break;
                    case DeviceFormat::F32: info.dataType = DataType::F32; break;
                    case DeviceFormat::F64: info.dataType = DataType::F64; break;
                    default: info.dataType = DataType::F32; break;
                    }
                    info.sampleRate = device.preferredSampleRate;
                }
            }
            catch (const std::exception& e)
            {
                std::stringstream ss;
                ss << "Cannot get default audio input device: " << e.what();
                _log(ss.str(), log::Type::Error);
            }
#endif // TLRENDER_AUDIO
        }

        void System::_run()
        {
            TLRENDER_P();
#if defined(TLRENDER_AUDIO)

            std::vector<DeviceInfo> devices = _getDevices();
            int defaultOutputDevice = -1;
            Info defaultOutputInfo;
            _getDefaultOutputDevice(devices, defaultOutputDevice, defaultOutputInfo);
            int defaultInputDevice = -1;
            Info defaultInputInfo;
            _getDefaultInputDevice(devices, defaultInputDevice, defaultInputInfo);

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
                ss << "    Default output device: " << defaultOutputDevice;
                _log(ss.str());
            }
            if (defaultOutputInfo != p.thread.defaultOutputInfo)
            {
                p.thread.defaultOutputInfo = defaultOutputInfo;

                std::stringstream ss;
                ss << "    Default output info: " <<
                    defaultOutputInfo.channelCount << " " <<
                    defaultOutputInfo.dataType << " " <<
                    defaultOutputInfo.sampleRate;
                _log(ss.str());
            }
            if (defaultInputDevice != p.thread.defaultInputDevice)
            {
                p.thread.defaultInputDevice = defaultInputDevice;

                std::stringstream ss;
                ss << "    Default input device: " << defaultInputDevice;
                _log(ss.str());
            }
            if (defaultInputInfo != p.thread.defaultInputInfo)
            {
                p.thread.defaultInputInfo = defaultInputInfo;

                std::stringstream ss;
                ss << "    Default input info: " <<
                    defaultInputInfo.channelCount << " " <<
                    defaultInputInfo.dataType << " " <<
                    defaultInputInfo.sampleRate;
                _log(ss.str());
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.devices = p.thread.devices;
                p.mutex.defaultOutputDevice = p.thread.defaultOutputDevice;
                p.mutex.defaultOutputInfo = p.thread.defaultOutputInfo;
                p.mutex.defaultInputDevice = p.thread.defaultInputDevice;
                p.mutex.defaultInputInfo = p.thread.defaultInputInfo;
            }
#endif // TLRENDER_AUDIO
        }
    }
}
