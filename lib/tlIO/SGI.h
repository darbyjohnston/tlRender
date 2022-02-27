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
        //! Silicon Graphics image I/O.
        //!
        //! References:
        //! - Paul Haeberli, "The SGI Image File Format, Version 1.00"
        //!   http://paulbourke.net/dataformats/sgirgb/sgiversion.html
        namespace sgi
        {
            //! SGI header.
            struct Header
            {
                uint16_t magic = 474;
                uint8_t  storage = 0;
                uint8_t  bytes = 0;
                uint16_t dimension = 0;
                uint16_t width = 0;
                uint16_t height = 0;
                uint16_t channels = 0;
                uint32_t pixelMin = 0;
                uint32_t pixelMax = 0;
            };

            //! SGI reader.
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

            //! SGI writer.
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
            };

            //! SGI plugin.
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
