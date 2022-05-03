// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/BMDDeviceSystem.h>

#include <tlDevice/BMDOutputDevice.h>

#include "platform.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace device
    {
        struct BMDDeviceSystem::Private
        {
            std::vector<DeviceInfo> deviceInfo;
            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        void BMDDeviceSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            IDeviceSystem::_init("tl::device::BMDDeviceSystem", context);

            TLRENDER_P();

            p.running = true;
            p.thread = std::thread(
                [this]
                {
                    TLRENDER_P();

#if defined(_WIN32)
                    CoInitialize(NULL);
#endif // _WIN32

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

                                dlstring_t dlstring;
                                dl->GetModelName(&dlstring);
                                deviceInfo.name = DlToStdString(dlstring);
                                DeleteString(dlstring);

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
                                            displayMode.name = DlToStdString(dlstring);
                                            DeleteString(dlstring);
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

                                dl->Release();

                                deviceInfoList.push_back(deviceInfo);
                            }
                        }
                        if (dlIterator)
                        {
                            dlIterator->Release();
                        }

                        {
                            std::unique_lock<std::mutex> lock(p.mutex);
                            p.deviceInfo = deviceInfoList;
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

        std::shared_ptr<IOutputDevice> BMDDeviceSystem::createDevice(int deviceIndex, int displayModeIndex)
        {
            std::shared_ptr<IOutputDevice> out;
            if (deviceIndex != -1 && displayModeIndex != -1)
            {
                if (auto context = getContext().lock())
                {
                    out = BMDOutputDevice::create(deviceIndex, displayModeIndex, context);
                }
            }
            return out;
        }

        void BMDDeviceSystem::tick()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> deviceInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
                deviceInfo = p.deviceInfo;
            }
            _deviceInfo->setIfChanged(deviceInfo);
        }
    }
}
