// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace device
    {
        //! Base class for output devices.
        class IOutputDevice : public std::enable_shared_from_this<IOutputDevice>
        {
            TLRENDER_NON_COPYABLE(IOutputDevice);

        protected:
            void _init(
                int deviceIndex,
                int displayModeIndex,
                const std::shared_ptr<system::Context>&);

            IOutputDevice();

        public:
            virtual ~IOutputDevice() = 0;

            //! Get the output device size.
            virtual const imaging::Size& getSize() const = 0;

            //! Get the output device frame rate.
            virtual const otime::RationalTime& getFrameRate() const = 0;

            //! Display an image.
            virtual void display(const std::shared_ptr<imaging::Image>&) = 0;

        protected:
            int _deviceIndex = 0;
            int _displayModeIndex = 0;
        };
    }
}
