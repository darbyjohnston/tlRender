// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/IOutputDevice.h>

namespace tl
{
    namespace device
    {
        //! BMD output device.
        class BMDOutputDevice : public IOutputDevice
        {
            TLRENDER_NON_COPYABLE(BMDOutputDevice);

        protected:
            void _init(
                int deviceIndex,
                int displayModeIndex,
                const std::shared_ptr<system::Context>&);

            BMDOutputDevice();

        public:
            ~BMDOutputDevice() override;

            //! Create a new BMD output device.
            static std::shared_ptr<BMDOutputDevice> create(
                int deviceIndex,
                int displayModeIndex,
                const std::shared_ptr<system::Context>&);

            const imaging::Size& getSize() const override;
            const otime::RationalTime& getFrameRate() const override;
            void display(const std::shared_ptr<imaging::Image>&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
