// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/AudioSystem.h>

#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>

#include <array>
#include <map>
#include <sstream>

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
            PaStream* portAudio;
            std::vector<std::string> apis;
            std::vector<Device> devices;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::audio::System", context);

            TLRENDER_P();

            {
                std::stringstream ss;
                ss << "PortAudio version: " << Pa_GetVersionText();
                _log(ss.str());
            }

            try
              {
                PaError err = Pa_Initialize();
                if ( err != paNoError )
                  {
                    _log( Pa_GetErrorText( err ), log::Type::Error );
                    throw "Could not initialize portaudio.";
                  }

                std::vector<std::string> log;
                log.push_back(std::string());
                const size_t paDeviceCount = Pa_GetDeviceCount();
                for (size_t i = 0; i < paDeviceCount; ++i)
                  {
                    const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
                    Device device;
                    device.name = info->name;
                    device.outputChannels = info->maxOutputChannels;
                    device.inputChannels = info->maxInputChannels;
                    device.preferredSampleRate = info->defaultSampleRate;
                    device.nativeFormats.push_back(DeviceFormat::S8);
                    device.nativeFormats.push_back(DeviceFormat::S16);
                    device.nativeFormats.push_back(DeviceFormat::S24);
                    device.nativeFormats.push_back(DeviceFormat::S32);
                    device.nativeFormats.push_back(DeviceFormat::F32);
                    p.devices.push_back(device);
                    {
                      std::stringstream ss;
                      ss << "    Device " << i << ": " << device.name;
                      log.push_back(ss.str());
                    }
                    {
                      std::stringstream ss;
                      ss << "        Channels: " <<
                        size_t(device.outputChannels) << " output, " <<
                        size_t(device.inputChannels) << " input";
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
                {
                  std::stringstream ss;
                  ss << "    Default input device: " << getDefaultInputDevice();
                  log.push_back(ss.str());
                }
                {
                  std::stringstream ss;
                  ss << "    Default output device: " << getDefaultOutputDevice();
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
        }

      System::System() :
        _p(new Private)
      {}

      System::~System()
      {
        PaError err = Pa_Terminate();
        if ( err != paNoError )
          {
            _log( Pa_GetErrorText( err ), log::Type::Error );
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

      const std::vector<Device>& System::getDevices() const
      {
        return _p->devices;
      }

      size_t System::getDefaultInputDevice() const
      {
        size_t out = Pa_GetDefaultInputDevice();
        return out;
      }

      size_t System::getDefaultOutputDevice() const
      {
        size_t out = Pa_GetDefaultOutputDevice();
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
    }
}
