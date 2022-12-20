// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/IOutputDevice.h>

namespace tl
{
    namespace device
    {
        void IOutputDevice::_init(
            int deviceIndex,
            int displayModeIndex,
            PixelType pixelType,
            const std::shared_ptr<system::Context>& context)
        {
            _deviceIndex = deviceIndex;
            _displayModeIndex = displayModeIndex;
            _pixelType = pixelType;
        }

        IOutputDevice::IOutputDevice()
        {}

        IOutputDevice::~IOutputDevice()
        {}

        int IOutputDevice::getDeviceIndex() const
        {
            return _deviceIndex;
        }

        int IOutputDevice::getDisplayModeIndex() const
        {
            return _displayModeIndex;
        }

        PixelType IOutputDevice::getPixelType() const
        {
            return _pixelType;
        }

        const imaging::Size& IOutputDevice::getSize() const
        {
            return _size;
        }

        const otime::RationalTime& IOutputDevice::getFrameRate() const
        {
            return _frameRate;
        }
        
        void IOutputDevice::setPlayback(timeline::Playback, const otime::RationalTime&)
        {}

        void IOutputDevice::setPixelData(const std::shared_ptr<PixelData>&)
        {}

        void IOutputDevice::setAudio(float, bool, double)
        {}

        void IOutputDevice::setAudioData(const std::vector<timeline::AudioData>&)
        {}
    }
}
