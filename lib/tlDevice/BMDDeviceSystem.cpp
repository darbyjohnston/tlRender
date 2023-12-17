// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDDeviceSystem.h>

#include <tlDevice/BMDOutputDevice.h>

#include <tlCore/Context.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include "platform.h"

#include <atomic>
#include <mutex>
#include <thread>

#if defined(__APPLE__)
typedef int64_t LONGLONG;
#elif defined(__linux__)
typedef bool BOOL;
typedef int64_t LONGLONG;
#endif // __APPLE__

namespace tl
{
    namespace device
    {
        struct BMDDeviceSystem::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::List<DeviceInfo> > deviceInfo;
            struct Mutex
            {
                std::vector<DeviceInfo> deviceInfo;
                std::mutex mutex;
            };
            Mutex mutex;
            std::thread thread;
            std::atomic<bool> running;
        };

        void BMDDeviceSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::device::BMDDeviceSystem", context);
            TLRENDER_P();

            p.context = context;

            p.deviceInfo = observer::List<DeviceInfo>::create();

            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLRENDER_P();

#if defined(_WIN32)
                    CoInitialize(NULL);
#endif // _WIN32

                    bool log = true;
                    while (p.running)
                    {
                        std::vector<DeviceInfo> deviceInfoList;

                        IDeckLinkIterator* dlIterator = nullptr;
                        if (GetDeckLinkIterator(&dlIterator) == S_OK)
                        {
                            IDeckLink* dl = nullptr;
                            while (dlIterator->Next(&dl) == S_OK)
                            {
                                DeviceInfo deviceInfo;

#if defined(__APPLE__)
                                CFStringRef dlstring;
                                dl->GetModelName(&dlstring);
                                StringToStdString(dlstring, deviceInfo.name);
                                CFRelease(dlstring);
#else // __APPLE__
                                dlstring_t dlstring;
                                dl->GetModelName(&dlstring);
                                deviceInfo.name = DlToStdString(dlstring);
                                DeleteString(dlstring);
#endif // __APPLE__

                                IDeckLinkOutput* dlOutput = nullptr;
                                if (dl->QueryInterface(IID_IDeckLinkOutput, (void**)&dlOutput) == S_OK)
                                {
                                    IDeckLinkDisplayModeIterator* dlDisplayModeIterator = nullptr;
                                    if (dlOutput->GetDisplayModeIterator(&dlDisplayModeIterator) == S_OK)
                                    {
                                        IDeckLinkDisplayMode* dlDisplayMode = nullptr;
                                        while (dlDisplayModeIterator->Next(&dlDisplayMode) == S_OK)
                                        {
                                            DisplayMode displayMode;
                                            dlDisplayMode->GetName(&dlstring);
#if defined(__APPLE__)
                                            StringToStdString(dlstring, displayMode.name);
                                            CFRelease(dlstring);
#else // __APPLE__
                                            displayMode.name = DlToStdString(dlstring);
                                            DeleteString(dlstring);
#endif // __APPLE__
                                            displayMode.size.w = dlDisplayMode->GetWidth();
                                            displayMode.size.h = dlDisplayMode->GetHeight();
                                            BMDTimeValue frameDuration;
                                            BMDTimeScale frameTimescale;
                                            dlDisplayMode->GetFrameRate(&frameDuration, &frameTimescale);
                                            displayMode.frameRate = otime::RationalTime(frameDuration, frameTimescale);

                                            dlDisplayMode->Release();

                                            deviceInfo.displayModes.push_back(displayMode);
                                        }
                                    }
                                    if (dlDisplayModeIterator)
                                    {
                                        dlDisplayModeIterator->Release();
                                    }
                                }
                                if (dlOutput)
                                {
                                    dlOutput->Release();
                                }

                                IDeckLinkProfileAttributes* dlProfileAttributes = nullptr;
                                if (dl->QueryInterface(IID_IDeckLinkProfileAttributes, (void**)&dlProfileAttributes) == S_OK)
                                {
                                    LONGLONG minVideoPreroll = 0;
                                    if (dlProfileAttributes->GetInt(BMDDeckLinkMinimumPrerollFrames, &minVideoPreroll) == S_OK)
                                    {
                                        deviceInfo.minVideoPreroll = minVideoPreroll;
                                    }
                                    BOOL hdrMetaData = false;
                                    if (dlProfileAttributes->GetFlag(BMDDeckLinkSupportsHDRMetadata, &hdrMetaData) == S_OK)
                                    {
                                        deviceInfo.hdrMetaData = hdrMetaData;
                                    }
                                    LONGLONG maxAudioChannels = 0;
                                    if (dlProfileAttributes->GetInt(BMDDeckLinkMaximumAudioChannels, &maxAudioChannels) == S_OK)
                                    {
                                        deviceInfo.maxAudioChannels = maxAudioChannels;
                                    }
                                }
                                dlProfileAttributes->Release();

                                dl->Release();

                                deviceInfo.pixelTypes.push_back(PixelType::_8BitBGRA);
                                deviceInfo.pixelTypes.push_back(PixelType::_10BitRGBXLE);

                                deviceInfoList.push_back(deviceInfo);
                            }
                        }
                        if (dlIterator)
                        {
                            dlIterator->Release();
                        }

                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            log = deviceInfoList != p.mutex.deviceInfo;
                            p.mutex.deviceInfo = deviceInfoList;
                        }

                        if (log)
                        {
                            log = false;
                            if (auto context = _context.lock())
                            {
                                for (const auto& i : deviceInfoList)
                                {
                                    std::vector<std::string> displayModes;
                                    for (const auto& j : i.displayModes)
                                    {
                                        displayModes.push_back(j.name);
                                    }
                                    context->log(
                                        "tl::device::BMDDeviceSystem",
                                        string::Format(
                                            "\n"
                                            "    {0}\n"
                                            "        Display modes: {1}\n"
                                            "        Min video preroll: {2}\n"
                                            "        HDR metadata: {3}\n"
                                            "        Max audio channels: {4}").
                                        arg(i.name).
                                        arg(string::join(displayModes, ", ")).
                                        arg(i.minVideoPreroll).
                                        arg(i.hdrMetaData).
                                        arg(i.maxAudioChannels));
                                }
                            }
                        }

                        time::sleep(getTickTime());
                    }

#if defined(_WIN32)
                    CoUninitialize();
#endif // _WIN32
                });
        }

        BMDDeviceSystem::BMDDeviceSystem() :
            _p(new Private)
        {}

        BMDDeviceSystem::~BMDDeviceSystem()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<BMDDeviceSystem> BMDDeviceSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<BMDDeviceSystem>(new BMDDeviceSystem);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IList<DeviceInfo> > BMDDeviceSystem::observeDeviceInfo() const
        {
            return _p->deviceInfo;
        }

        void BMDDeviceSystem::tick()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> deviceInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                deviceInfo = p.mutex.deviceInfo;
            }
            p.deviceInfo->setIfChanged(deviceInfo);
        }

        std::chrono::milliseconds BMDDeviceSystem::getTickTime() const
        {
            return std::chrono::milliseconds(1000);
        }
    }
}
