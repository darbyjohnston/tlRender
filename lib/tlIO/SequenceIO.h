// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Plugin.h>

namespace tl
{
    namespace io
    {
        //! Default speed for image sequences.
        const float sequenceDefaultSpeed = 24.F;

        //! Number of threads.
        const size_t sequenceThreadCount = 16;

        //! Timeout for requests.
        const std::chrono::milliseconds sequenceRequestTimeout(5);

        //! Base class for image sequence readers.
        class ISequenceRead : public IRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            ISequenceRead();

        public:
            virtual ~ISequenceRead();

            std::future<Info> getInfo() override;
            std::future<VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const Options& = Options()) override;
            void cancelRequests() override;

        protected:
            virtual Info _getInfo(
                const std::string& fileName,
                const dtk::InMemoryFile*) = 0;
            virtual VideoData _readVideo(
                const std::string& fileName,
                const dtk::InMemoryFile*,
                const OTIO_NS::RationalTime&,
                const Options&) = 0;

            //! \bug This must be called in the sub-class destructor.
            void _finish();

            int64_t _startFrame = 0;
            int64_t _endFrame = 0;
            float _defaultSpeed = sequenceDefaultSpeed;

        private:
            void _thread();
            void _finishRequests();
            void _cancelRequests();

            DTK_PRIVATE();
        };

        //! Base class for image sequence writers.
        class ISequenceWrite : public IWrite
        {
        protected:
            void _init(
                const file::Path&,
                const Info&,
                const Options&,
                const std::shared_ptr<dtk::LogSystem>&);

            ISequenceWrite();

        public:
            virtual ~ISequenceWrite();

            void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<dtk::Image>&,
                const Options& = Options()) override;

        protected:
            virtual void _writeVideo(
                const std::string& fileName,
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<dtk::Image>&,
                const Options&) = 0;

        private:
            DTK_PRIVATE();
        };
    }
}
