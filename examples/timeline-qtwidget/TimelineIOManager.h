// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Context.h>
#include <tlCore/LRUCache.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! TimelineI/O manager.
            class TimelineIOManager : public std::enable_shared_from_this<TimelineIOManager>
            {
            protected:
                void _init(
                    const io::Options&,
                    const std::shared_ptr<system::Context>&);

                TimelineIOManager();

            public:
                ~TimelineIOManager();

                //! Create a new timeline I/O manager.
                static std::shared_ptr<TimelineIOManager> create(
                    const io::Options&,
                    const std::shared_ptr<system::Context>&);

                //! Get information.
                std::future<io::Info> getInfo(const file::Path&);

                //! Read video data.
                std::future<io::VideoData> readVideo(
                    const file::Path&,
                    const otime::RationalTime&,
                    uint16_t layer = 0);

                //! Read audio data.
                std::future<io::AudioData> readAudio(
                    const file::Path&,
                    const otime::TimeRange&);

                //! Cancel pending requests.
                void cancelRequests();

                //! Observe when pending requests are canceled.
                std::shared_ptr<observer::IValue<bool> > observeCancelRequests() const;

            private:
                std::weak_ptr<system::Context> _context;
                io::Options _ioOptions;
                memory::LRUCache<std::string, std::shared_ptr<io::IRead> > _cache;
                std::shared_ptr<observer::Value<bool> > _cancelRequests;
            };
        }
    }
}
