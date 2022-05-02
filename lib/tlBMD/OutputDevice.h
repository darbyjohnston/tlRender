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

    namespace bmd
    {
        //! Output device.
        class OutputDevice : public std::enable_shared_from_this<OutputDevice>
        {
            TLRENDER_NON_COPYABLE(OutputDevice);

        protected:
            void _init(
                int deviceIndex,
                int displayModeIndex,
                const std::shared_ptr<system::Context>&);

            OutputDevice();

        public:
            ~OutputDevice();

            //! Create a new output device.
            static std::shared_ptr<OutputDevice> create(
                int deviceIndex,
                int displayModeIndex,
                const std::shared_ptr<system::Context>&);

            //! Get the output device size.
            const imaging::Size& getSize() const;

            //! Get the output device frame rate.
            const otime::RationalTime& getFrameRate() const;

            //! Display an image.
            void display(const std::shared_ptr<imaging::Image>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
