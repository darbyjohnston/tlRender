// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlBMD/DeviceSystem.h>

#include "platform.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace tl
{
    namespace bmd
    {
        bool DisplayMode::operator == (const DisplayMode& other) const
        {
            return
                displayMode == other.displayMode &&
                size == other.size &&
                frameRate == other.frameRate;
        }

        bool DeviceInfo::operator == (const DeviceInfo& other) const
        {
            return
                model == other.model &&
                displayModes == other.displayModes;
        }

        struct DeviceSystem::Private
        {
            std::shared_ptr<observer::List<DeviceInfo> > deviceInfo;
            std::vector<DeviceInfo> deviceInfoThread;
            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running;
        };

        void DeviceSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::bmd::DeviceSystem", context);
        }

        DeviceSystem::DeviceSystem() :
            _p(new Private)
        {
            TLRENDER_P();

            p.deviceInfo = observer::List<DeviceInfo>::create();

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

                                dlstring_t modelName;
                                dl->GetModelName(&modelName);
                                deviceInfo.model = DlToStdString(modelName);
                                DeleteString(modelName);

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
                                            displayMode.displayMode = dlDisplayMode->GetDisplayMode();
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
                            p.deviceInfoThread = deviceInfoList;
                        }

                        time::sleep(getTickTime());
                    }

#if defined(_WIN32)
                    CoUninitialize();
#endif // _WIN32
                });
        }

        DeviceSystem::~DeviceSystem()
        {
            TLRENDER_P();
            p.running = false;
            if (p.thread.joinable())
            {
                p.thread.join();
            }
        }

        std::shared_ptr<DeviceSystem> DeviceSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<DeviceSystem>(new DeviceSystem);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IList<DeviceInfo> > DeviceSystem::observeDeviceInfo() const
        {
            return _p->deviceInfo;
        }

        void DeviceSystem::tick()
        {
            TLRENDER_P();
            std::vector<DeviceInfo> deviceInfo;
            {
                std::unique_lock<std::mutex> lock(p.mutex);
            }
            p.deviceInfo->setIfChanged(deviceInfo);
        }

        std::chrono::milliseconds DeviceSystem::getTickTime() const
        {
            return std::chrono::milliseconds(1000);
        }
    }
}
