// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

extern "C"
{
#include <jpeglib.h>
}

#include <setjmp.h>

namespace tl
{
    //! JPEG image I/O.
    namespace jpeg
    {
        //! JPEG error.
        struct ErrorStruct
        {
            struct jpeg_error_mgr pub;
            std::vector<std::string> messages;
            jmp_buf jump;
        };

        //! JPEG error function.
        void errorFunc(j_common_ptr);

        //! JPEG warning function.
        void warningFunc(j_common_ptr, int level);

        //! JPEG reader.
        class Read : public io::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

        protected:
            io::Info _getInfo(
                const std::string& fileName,
                const dtk::InMemoryFile*) override;
            io::VideoData _readVideo(
                const std::string& fileName,
                const dtk::InMemoryFile*,
                const OTIO_NS::RationalTime&,
                const io::Options&) override;
        };

        //! JPEG writer.
        class Write : public io::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<dtk::LogSystem>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<dtk::LogSystem>&);

        protected:
            void _writeVideo(
                const std::string& fileName,
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<dtk::Image>&,
                const io::Options&) override;

        private:
            int _quality = 90;
        };

        //! JPEG plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options & = io::Options()) override;
            dtk::ImageInfo getWriteInfo(
                const dtk::ImageInfo&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;
        };
    }
}
