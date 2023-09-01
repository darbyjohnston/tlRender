// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/Context.h>
#include <tlCore/FileIO.h>
#include <tlCore/ISystem.h>
#include <tlCore/Mesh.h>
#include <tlCore/Path.h>

#include <future>

namespace tl
{
    namespace ui
    {
        //! Information request.
        struct InfoRequest
        {
            uint64_t id;
            std::future<io::Info> future;
        };

        //! Video thumbnail request.
        struct ThumbnailRequest
        {
            uint64_t id;
            std::future<std::shared_ptr<image::Image> > future;
        };

        //! Audio waveform request.
        struct WaveformRequest
        {
            uint64_t id;
            std::future<std::shared_ptr<geom::TriangleMesh2> > future;
        };

        //! Thumbnail system.
        class ThumbnailSystem : public system::ISystem
        {
        protected:
            void _init(const std::shared_ptr<system::Context>&);

            ThumbnailSystem();

        public:
            ~ThumbnailSystem();

            //! Create a new system.
            static std::shared_ptr<ThumbnailSystem> create(
                const std::shared_ptr<system::Context>&);

            //! Get information.
            InfoRequest getInfo(const file::Path&);

            //! Get information.
            InfoRequest getInfo(
                const file::Path&,
                const std::vector<file::MemoryRead>&);

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                int height,
                const file::Path&,
                const otime::RationalTime& = time::invalidTime,
                uint16_t layer = 0);

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                int height,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::RationalTime& = time::invalidTime,
                uint16_t layer = 0);

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                const math::Size2i&,
                const file::Path&,
                const otime::TimeRange& = time::invalidTimeRange);

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                const math::Size2i&,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const otime::TimeRange& = time::invalidTimeRange);

            //! Cancel pending requests.
            void cancelRequests(std::vector<uint64_t>);

        private:
            void _run();
            void _cancelRequests();
            
            TLRENDER_PRIVATE();
        };
    }
}
