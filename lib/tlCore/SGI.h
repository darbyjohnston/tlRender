// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/FileIO.h>
#include <tlCore/SequenceIO.h>

namespace tl
{
    //! Silicon Graphics image I/O.
    //!
    //! References:
    //! - Paul Haeberli, "The SGI Image File Format, Version 1.00"
    //!   http://paulbourke.net/dataformats/sgirgb/sgiversion.html
    namespace sgi
    {
        struct Header
        {
            uint16_t magic     = 474;
            uint8_t  storage   = 0;
            uint8_t  bytes     = 0;
            uint16_t dimension = 0;
            uint16_t width     = 0;
            uint16_t height    = 0;
            uint16_t channels  = 0;
            uint32_t pixelMin  = 0;
            uint32_t pixelMax  = 0;
        };

        //! SGI reader.
        class Read : public avio::ISequenceRead
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);

        protected:
            avio::Info _getInfo(const std::string& fileName) override;
            avio::VideoData _readVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                uint16_t layer) override;
        };

        //! SGI writer.
        class Write : public avio::ISequenceWrite
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);
            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);

        protected:
            void _writeVideo(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;
        };

        //! SGI plugin.
        class Plugin : public avio::IPlugin
        {
        protected:
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::shared_ptr<core::LogSystem>&);

            std::shared_ptr<avio::IRead> read(
                const file::Path&,
                const avio::Options& = avio::Options()) override;
            std::vector<imaging::PixelType> getWritePixelTypes() const override;
            memory::Endian getWriteEndian() const override;
            std::shared_ptr<avio::IWrite> write(
                const file::Path&,
                const avio::Info&,
                const avio::Options& = avio::Options()) override;
        };
    }
}
