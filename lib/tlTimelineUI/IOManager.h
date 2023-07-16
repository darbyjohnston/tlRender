// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Context.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace timelineui
    {
        //! I/O manager.
        //! 
        //! \todo Serialize requests to control the number of threads used.
        class IOManager : public std::enable_shared_from_this<IOManager>
        {
        protected:
            void _init(
                const io::Options&,
                const std::shared_ptr<system::Context>&);

            IOManager();

        public:
            ~IOManager();

            //! Create a new I/O manager.
            static std::shared_ptr<IOManager> create(
                const io::Options&,
                const std::shared_ptr<system::Context>&);

            //! Get information.
            std::future<io::Info> getInfo(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::RationalTime& startTime);

            //! Read video data.
            std::future<io::VideoData> readVideo(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::RationalTime&,
                uint16_t layer = 0);

            //! Read audio data.
            std::future<io::AudioData> readAudio(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::TimeRange&);

            //! Cancel pending requests.
            void cancelRequests();

            //! Observe when pending requests are canceled.
            std::shared_ptr<observer::IValue<bool> > observeCancelRequests() const;

        private:
            void _run();
            void _cancelRequests();
            
            TLRENDER_PRIVATE();
        };
    }
}
