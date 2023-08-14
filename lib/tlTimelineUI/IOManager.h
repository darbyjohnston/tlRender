// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Context.h>
#include <tlCore/Mesh.h>
#include <tlCore/Size.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace timelineui
    {
        //! I/O manager.
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

            //! Request information.
            std::future<io::Info> requestInfo(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::RationalTime& startTime);

            //! Request video thumbnails.
            std::future<std::shared_ptr<image::Image> > requestVideo(
                const math::Size2i&,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::RationalTime& startTime,
                const otime::RationalTime&,
                uint16_t layer = 0);

            //! Request audio waveforms.
            std::future<std::shared_ptr<geom::TriangleMesh2> > requestAudio(
                const math::Size2i&,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::RationalTime& startTime,
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
