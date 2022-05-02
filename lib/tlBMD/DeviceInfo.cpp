// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlBMD/DeviceInfo.h>

#include "platform.h"

namespace tl
{
    namespace bmd
    {
        namespace
        {
            std::chrono::milliseconds infoTimeout = std::chrono::milliseconds(1000);
        }

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

        std::future<std::vector<DeviceInfo> > getInfo()
        {
            return std::async(
                std::launch::async,
                []() -> std::vector<DeviceInfo>
                {
#if defined(_WIN32)
                    CoInitialize(NULL);
#endif // _WIN32

                    std::vector<DeviceInfo> out;

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

                            out.push_back(deviceInfo);
                        }
                    }
                    if (dlIterator)
                    {
                        dlIterator->Release();
                    }

#if defined(_WIN32)
                    CoUninitialize();
#endif // _WIN32

                    return out;
                });
        }
    }
}
