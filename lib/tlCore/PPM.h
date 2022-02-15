// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/FileIO.h>
#include <tlCore/SequenceIO.h>

namespace tl
{
    //! NetPBM I/O.
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
        TLRENDER_ENUM(Data);
        TLRENDER_ENUM_SERIALIZE(Data);

        //! Get the number of bytes in a file scanline.
        size_t getFileScanlineByteCount(
            int    width,
            size_t channelCount,
            size_t bitDepth);

        //! Read PPM file ASCII data.
        void readASCII(
            const std::shared_ptr<file::FileIO>& io,
            uint8_t*                             out,
            size_t                               size,
            size_t                               componentSize);

        //! Save PPM file ASCII data.
        size_t writeASCII(
            const uint8_t* in,
            char*          out,
            size_t         size,
            size_t         componentSize);

        //! PPM reader.
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

        //! PPM writer.
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

        private:
            Data _data = Data::Binary;
        };

        //! PPM plugin.
        class Plugin : public avio::IPlugin
        {
        protected:
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
