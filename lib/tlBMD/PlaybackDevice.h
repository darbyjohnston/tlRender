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
        //! Playback device.
        class PlaybackDevice : public std::enable_shared_from_this<PlaybackDevice>
        {
            TLRENDER_NON_COPYABLE(PlaybackDevice);

        protected:
            void _init(
                int deviceIndex,
                const std::shared_ptr<system::Context>&);

            PlaybackDevice();

        public:
            ~PlaybackDevice();

            //! Create a new playback device.
            static std::shared_ptr<PlaybackDevice> create(
                int deviceIndex,
                const std::shared_ptr<system::Context>&);

            //! Get the playback device size.
            const imaging::Size& getSize() const;

            //! Get the playback device frame rate.
            const otime::RationalTime& getFrameRate() const;

            //! Display an image for playback.
            void display(const std::shared_ptr<imaging::Image>&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
