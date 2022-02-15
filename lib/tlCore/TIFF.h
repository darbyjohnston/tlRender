// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/SequenceIO.h>

namespace tl
{
    //! TIFF I/O.
    namespace tiff
    {
        //! TIFF reader.
        class Read : public avio::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Options&,
                const std::weak_ptr<core::LogSystem>&);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const avio::Options&,
                const std::weak_ptr<core::LogSystem>&);

        protected:
            avio::Info _getInfo(const std::string& fileName) override;
            avio::VideoData _readVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer) override;
        };

        //! TIFF writer.
        class Write : public avio::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::weak_ptr<core::LogSystem>&);
            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::weak_ptr<core::LogSystem>&);

        protected:
            void _writeVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;
        };

        //! TIFF plugin.
        class Plugin : public avio::IPlugin
        {
        protected:
            void _init(const std::weak_ptr<core::LogSystem>&);
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::weak_ptr<core::LogSystem>&);

            std::shared_ptr<avio::IRead> read(
                const file::Path&,
                const avio::Options& = avio::Options()) override;
            imaging::Info getWriteInfo(
                const imaging::Info&,
                const avio::Options& = avio::Options()) const override;
            std::shared_ptr<avio::IWrite> write(
                const file::Path&,
                const avio::Info&,
                const avio::Options& = avio::Options()) override;
        };
    }
}
