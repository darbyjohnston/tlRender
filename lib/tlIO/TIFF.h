// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

namespace tl
{
    //! TIFF I/O.
    namespace tiff
    {
        //! TIFF reader.
        class Read : public io::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

        protected:
            io::Info _getInfo(
                const std::string& fileName,
                const file::MemoryRead*) override;
            io::VideoData _readVideo(
                const std::string& fileName,
                const file::MemoryRead*,
                const otime::RationalTime&,
                uint16_t layer) override;
        };

        //! TIFF writer.
        class Write : public io::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

        protected:
            void _writeVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;
        };

        //! TIFF plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(const std::weak_ptr<log::System>&);

            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::weak_ptr<log::System>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options & = io::Options()) override;
            imaging::Info getWriteInfo(
                const imaging::Info&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;
        };
    }
}
