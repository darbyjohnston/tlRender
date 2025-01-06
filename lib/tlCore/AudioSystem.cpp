// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlCore/AudioSystem.h>

#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#if defined(TLRENDER_SDL2)
#include <SDL2/SDL.h>
#endif // TLRENDER_SDL2
#if defined(TLRENDER_SDL3)
#include <SDL3/SDL.h>
#endif // TLRENDER_SDL3

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

        bool DeviceInfo::operator == (const DeviceInfo& other) const
        {
            return
                id == other.id &&
                info == other.info;
        }

        bool DeviceInfo::operator != (const DeviceInfo& other) const
        {
            return !(*this == other);
        }

        struct System::Private
        {
            bool init = false;
            std::vector<std::string> drivers;
            std::shared_ptr<observer::List<DeviceInfo> > devices;
            std::shared_ptr<observer::Value<DeviceInfo> > defaultDevice;

            struct Mutex
            {
                std::vector<DeviceInfo> devices;
                DeviceInfo defaultDevice;
                std::mutex mutex;
            };
            Mutex mutex;
            struct Thread
            {
                std::vector<DeviceInfo> devices;
                DeviceInfo defaultDevice;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::audio::System", context);
            TLRENDER_P();

#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
#if defined(TLRENDER_SDL2)
            p.init = SDL_Init(SDL_INIT_AUDIO) >= 0;
#elif defined(TLRENDER_SDL3)
            p.init = SDL_Init(SDL_INIT_AUDIO);
#endif // TLRENDER_SDL2
            if (!p.init)
            {
                std::stringstream ss;
                ss << "Cannot initialize SDL2";
                _log(ss.str(), log::Type::Error);
            }
            if (p.init)
            {
                const int count = SDL_GetNumAudioDrivers();
                for (int i = 0; i < count; ++i)
                {
                    p.drivers.push_back(SDL_GetAudioDriver(i));
                }
                {
                    std::stringstream ss;
                    ss << "Audio drivers: " << string::join(p.drivers, ", ");
                    _log(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "Current audio driver: " << SDL_GetCurrentAudioDriver();
                    _log(ss.str());
                }
            }
#endif // TLRENDER_SDL2

            const std::vector<DeviceInfo> devices = _getDevices();
            const DeviceInfo defaultDevice = _getDefaultDevice();

            p.devices = observer::List<DeviceInfo>::create(devices);
            p.defaultDevice = observer::Value<DeviceInfo>::create(defaultDevice);

            p.mutex.devices = devices;
            p.mutex.defaultDevice = defaultDevice;

#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
            if (p.init)
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
#endif // TLRENDER_SDL2
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
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
            SDL_Quit();
#endif // TLRENDER_SDL2
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

        const std::vector<std::string>& System::getDrivers() const
        {
            return _p->drivers;
        }

        const std::vector<DeviceInfo>& System::getDevices() const
        {
            return _p->devices->get();
        }

        std::shared_ptr<observer::IList<DeviceInfo> > System::observeDevices() const
        {
            return _p->devices;
        }

        DeviceInfo System::getDefaultDevice() const
        {
            return _p->defaultDevice->get();
        }

        std::shared_ptr<observer::IValue<DeviceInfo> > System::observeDefaultDevice() const
        {
            return _p->defaultDevice;
        }

        void System::tick()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> devices;
            DeviceInfo defaultDevice;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                devices = p.mutex.devices;
                defaultDevice = p.mutex.defaultDevice;
            }
            p.devices->setIfChanged(devices);
            p.defaultDevice->setIfChanged(defaultDevice);
        }

        std::chrono::milliseconds System::getTickTime() const
        {
            return std::chrono::milliseconds(500);
        }

        namespace
        {
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)
            //! \todo This is duplicated in AudioSystem.cpp and PlayerAudio.cpp
            audio::DataType fromSDL(SDL_AudioFormat value)
            {
                audio::DataType out = audio::DataType::F32;
                if (SDL_AUDIO_BITSIZE(value) == 8 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    !SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::S8;
                }
                else if (SDL_AUDIO_BITSIZE(value) == 16 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    !SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::S16;
                }
                else if (SDL_AUDIO_BITSIZE(value) == 32 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    !SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::S32;
                }
                else if (SDL_AUDIO_BITSIZE(value) == 32 &&
                    SDL_AUDIO_ISSIGNED(value) &&
                    SDL_AUDIO_ISFLOAT(value))
                {
                    out = audio::DataType::F32;
                }
                return out;
            }
#endif // TLRENDER_SDL2
        }

        std::vector<DeviceInfo> System::_getDevices()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> out;
#if defined(TLRENDER_SDL2)
            const int count = SDL_GetNumAudioDevices(0);
            for (int i = 0; i < count; ++i)
            {
                DeviceInfo device;
                device.id.number = i;
                device.id.name = SDL_GetAudioDeviceName(i, 0);
                device.info.channelCount = 2;
                device.info.dataType = audio::DataType::F32;
                device.info.sampleRate = 48000;
                out.push_back(device);
            }
#elif defined(TLRENDER_SDL3)
            int count = 0;
            SDL_AudioDeviceID* ids = SDL_GetAudioPlaybackDevices(&count);
            for (int i = 0; i < count; ++i)
            {
                DeviceInfo device;
                device.id.number = ids[i];
                device.id.name = SDL_GetAudioDeviceName(ids[i]);
                SDL_AudioSpec spec;
                int sampleFrames = 0;
                SDL_GetAudioDeviceFormat(ids[i], &spec, &sampleFrames);
                device.info.channelCount = spec.channels;
                device.info.dataType = fromSDL(spec.format);
                device.info.sampleRate = spec.freq;
                out.push_back(device);
            }
            SDL_free(ids);
#endif // TLRENDER_SDL2
            return out;
        }

        DeviceInfo System::_getDefaultDevice()
        {
            DeviceInfo out;
#if defined(TLRENDER_SDL2)
            out.info.channelCount = 2;
            out.info.dataType = audio::DataType::F32;
            out.info.sampleRate = 48000;
#elif defined(TLRENDER_SDL3)
            out.id.name = "Default";
            SDL_AudioSpec spec;
            int sampleFrames = 0;
            SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &sampleFrames);
            out.info.channelCount = spec.channels;
            out.info.dataType = fromSDL(spec.format);
            out.info.sampleRate = spec.freq;
#endif // TLRENDER_SDL2
            return out;
        }

        void System::_run()
        {
            TLRENDER_P();
#if defined(TLRENDER_SDL2) || defined(TLRENDER_SDL3)

            const std::vector<DeviceInfo> devices = _getDevices();
            const DeviceInfo defaultDevice = _getDefaultDevice();

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
                        ss << "    Device: " << device.id.number << " " << device.id.name << "\n" <<
                            "      Channels: " << device.info.channelCount << "\n" <<
                            "      Data type: " << device.info.dataType << "\n" <<
                            "      Sample rate: " << device.info.sampleRate;
                        log.push_back(ss.str());
                    }
                }
                _log(string::join(log, "\n"));
            }
            if (defaultDevice != p.thread.defaultDevice)
            {
                p.thread.defaultDevice = defaultDevice;

                std::stringstream ss;
                ss << "Default device: " << defaultDevice.id.number << " " << defaultDevice.id.name << "\n" <<
                    "      Channels: " << defaultDevice.info.channelCount << "\n" <<
                    "      Data type: " << defaultDevice.info.dataType << "\n" <<
                    "      Sample rate: " << defaultDevice.info.sampleRate;
                _log(ss.str());
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.devices = p.thread.devices;
                p.mutex.defaultDevice = p.thread.defaultDevice;
            }
#endif // TLRENDER_SDL2
        }
    }
}
