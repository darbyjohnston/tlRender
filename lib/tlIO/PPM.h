// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

#include <feather-tk/core/Util.h>

namespace tl
{
    //! NetPBM image I/O.
    //!
    //! References:
    //! - Netpbm, "PPM Format Specification"
    //!   http://netpbm.sourceforge.net/doc/ppm.html
    namespace ppm
    {
        //! PPM data type.
        enum class Data
        {
            ASCII,
            Binary,

            Count,
            First = ASCII
        };
        FTK_ENUM(Data);

        //! Get the number of bytes in a file scanline.
        size_t getFileScanlineByteCount(
            int    width,
            size_t channelCount,
            size_t bitDepth);

        //! Read PPM file ASCII data.
        void readASCII(
            const std::shared_ptr<ftk::FileIO>& io,
            uint8_t* out,
            size_t size,
            size_t componentSize);

        //! Save PPM file ASCII data.
        size_t writeASCII(
            const uint8_t* in,
            char* out,
            size_t size,
            size_t componentSize);

        //! PPM reader.
        class Read : public io::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<ftk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const std::vector<ftk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

        protected:
            io::Info _getInfo(
                const std::string& fileName,
                const ftk::InMemoryFile*) override;
            io::VideoData _readVideo(
                const std::string& fileName,
                const ftk::InMemoryFile*,
                const OTIO_NS::RationalTime&,
                const io::Options&) override;
        };

        //! PPM writer.
        class Write : public io::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

        protected:
            void _writeVideo(
                const std::string& fileName,
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<ftk::Image>&,
                const io::Options&) override;

        private:
            Data _data = Data::Binary;
        };

        //! PPM read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            ReadPlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<ftk::InMemoryFile>&,
                const io::Options & = io::Options()) override;
        };

        //! PPM write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            WritePlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            ftk::ImageInfo getInfo(
                const ftk::ImageInfo&,
                const io::Options & = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options & = io::Options()) override;
        };
    }
}
