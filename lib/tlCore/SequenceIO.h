// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/AVIO.h>

namespace tl
{
    namespace avio
    {
        //! Default speed for image sequences.
        const float sequenceDefaultSpeed = 24.F;

        //! Number of threads.
        const size_t sequenceThreadCount = 16;

        //! Timeout for requests.
        const std::chrono::milliseconds sequenceRequestTimeout(1);

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
            std::future<VideoData> readVideo(const otime::RationalTime&, uint16_t layer = 0) override;
            bool hasRequests() override;
            void cancelRequests() override;
            void stop() override;
            bool hasStopped() const override;

        protected:
            virtual Info _getInfo(const std::string& fileName) = 0;
            virtual VideoData _readVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer) = 0;

            //! This must be called in the sub-class destructor.
            void _finish();
            
            int64_t _startFrame = 0;
            int64_t _endFrame = 0;
            float _defaultSpeed = sequenceDefaultSpeed;

        private:
            void _run();

            TLRENDER_PRIVATE();
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

            void writeVideo(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;

        protected:
            virtual void _writeVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) = 0;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
