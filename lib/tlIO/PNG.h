// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

namespace tl
{
    //! PNG image I/O.
    namespace png
    {
        //! PNG reader.
        class Read : public io::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

        protected:
            io::Info _getInfo(
                const std::string& fileName,
                const feather_tk::InMemoryFile*) override;
            io::VideoData _readVideo(
                const std::string& fileName,
                const feather_tk::InMemoryFile*,
                const OTIO_NS::RationalTime&,
                const io::Options&) override;
        };

        //! PNG writer.
        class Write : public io::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

        protected:
            void _writeVideo(
                const std::string& fileName,
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<feather_tk::Image>&,
                const io::Options&) override;
        };

        //! PNG read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            ReadPlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<feather_tk::LogSystem>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const io::Options & = io::Options()) override;
        };

        //! PNG write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            WritePlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<feather_tk::LogSystem>&);

            feather_tk::ImageInfo getInfo(
                const feather_tk::ImageInfo&,
                const io::Options & = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options & = io::Options()) override;
        };
    }
}
