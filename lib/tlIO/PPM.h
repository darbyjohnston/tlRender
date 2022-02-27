// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

#include <tlCore/FileIO.h>

namespace tl
{
    namespace io
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
                const std::shared_ptr<core::file::FileIO>& io,
                uint8_t* out,
                size_t                               size,
                size_t                               componentSize);

            //! Save PPM file ASCII data.
            size_t writeASCII(
                const uint8_t* in,
                char* out,
                size_t         size,
                size_t         componentSize);

            //! PPM reader.
            class Read : public ISequenceRead
            {
            protected:
                void _init(
                    const core::file::Path&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);
                Read();

            public:
                ~Read() override;

                //! Create a new reader.
                static std::shared_ptr<Read> create(
                    const core::file::Path&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);

            protected:
                Info _getInfo(const std::string& fileName) override;
                VideoData _readVideo(
                    const std::string& fileName,
                    const otime::RationalTime&,
                    uint16_t layer) override;
            };

            //! PPM writer.
            class Write : public ISequenceWrite
            {
            protected:
                void _init(
                    const core::file::Path&,
                    const Info&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);
                Write();

            public:
                ~Write() override;

                //! Create a new writer.
                static std::shared_ptr<Write> create(
                    const core::file::Path&,
                    const Info&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);

            protected:
                void _writeVideo(
                    const std::string& fileName,
                    const otime::RationalTime&,
                    const std::shared_ptr<core::imaging::Image>&) override;

            private:
                Data _data = Data::Binary;
            };

            //! PPM plugin.
            class Plugin : public IPlugin
            {
            protected:
                Plugin();

            public:
                //! Create a new plugin.
                static std::shared_ptr<Plugin> create(const std::weak_ptr<core::log::System>&);

                std::shared_ptr<IRead> read(
                    const core::file::Path&,
                    const Options & = Options()) override;
                core::imaging::Info getWriteInfo(
                    const core::imaging::Info&,
                    const Options & = Options()) const override;
                std::shared_ptr<IWrite> write(
                    const core::file::Path&,
                    const Info&,
                    const Options & = Options()) override;
            };
        }
    }
}
