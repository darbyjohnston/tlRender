// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AVIO.h>

namespace tlr
{
    namespace avio
    {
        //! Default speed for image sequences.
        const float sequenceDefaultSpeed = 24.F;

        //! Number of threads.
        const size_t sequenceThreadCount = 4;

        //! Timeout for frame requests.
        const std::chrono::microseconds sequenceRequestTimeout(1000);

        //! Base class for image sequence readers.
        class ISequenceRead : public IRead
        {
        protected:
            void _init(
                const file::Path&,
                const Options&,
                const std::shared_ptr<core::LogSystem>&);
            ISequenceRead();

        public:
            ~ISequenceRead() override;

            std::future<Info> getInfo() override;
            std::future<VideoFrame> readVideoFrame(const otime::RationalTime&) override;
            bool hasVideoFrames() override;
            void cancelVideoFrames() override;
            void stop() override;
            bool hasStopped() const override;

        protected:
            virtual Info _getInfo(const std::string& fileName) = 0;
            virtual VideoFrame _readVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&) = 0;

        private:
            void _run();

            TLR_PRIVATE();
        };

        //! Base class for image sequence writers.
        class ISequenceWrite : public IWrite
        {
        protected:
            void _init(
                const file::Path&,
                const Info&,
                const Options&,
                const std::shared_ptr<core::LogSystem>&);
            ISequenceWrite();

        public:
            ~ISequenceWrite() override;

            void writeVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;

        protected:
            virtual void _writeVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) = 0;

        private:
            TLR_PRIVATE();
        };
    }
}
