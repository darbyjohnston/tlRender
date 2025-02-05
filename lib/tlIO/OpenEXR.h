// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

namespace tl
{
    //! OpenEXR image I/O.
    namespace exr
    {
        //! Channel grouping.
        enum class ChannelGrouping
        {
            None,
            Known,
            All,

            Count,
            First = None
        };
        TLRENDER_ENUM(ChannelGrouping);
        TLRENDER_ENUM_SERIALIZE(ChannelGrouping);

        //! Compression types.
        enum class Compression
        {
            None,
            RLE,
            ZIPS,
            ZIP,
            PIZ,
            PXR24,
            B44,
            B44A,
            DWAA,
            DWAB,

            Count,
            First = None
        };
        TLRENDER_ENUM(Compression);
        TLRENDER_ENUM_SERIALIZE(Compression);

        //! OpenEXR reader.
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

        private:
            ChannelGrouping _channelGrouping = ChannelGrouping::Known;
        };

        //! OpenEXR writer.
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
            Compression _compression = Compression::ZIP;
            float _dwaCompressionLevel = 45.F;
        };

        //! OpenEXR plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

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
                const io::Options& = io::Options()) override;
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
