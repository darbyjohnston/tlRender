// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/SequenceIO.h>

extern "C"
{
#include <jpeglib.h>

} // extern "C"

#include <setjmp.h>

namespace tl
{
    namespace io
    {
        //! JPEG I/O.
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
            class Read : public ISequenceRead
            {
            protected:
                void _init(
                    const core::file::Path&,
                    const Options&,
                    const std::weak_ptr<core::LogSystem>&);
                Read();

            public:
                ~Read() override;

                //! Create a new reader.
                static std::shared_ptr<Read> create(
                    const core::file::Path&,
                    const Options&,
                    const std::weak_ptr<core::LogSystem>&);

            protected:
                Info _getInfo(const std::string& fileName) override;
                VideoData _readVideo(
                    const std::string& fileName,
                    const otime::RationalTime&,
                    uint16_t layer) override;
            };

            //! JPEG writer.
            class Write : public ISequenceWrite
            {
            protected:
                void _init(
                    const core::file::Path&,
                    const Info&,
                    const Options&,
                    const std::weak_ptr<core::LogSystem>&);
                Write();

            public:
                ~Write() override;

                //! Create a new writer.
                static std::shared_ptr<Write> create(
                    const core::file::Path&,
                    const Info&,
                    const Options&,
                    const std::weak_ptr<core::LogSystem>&);

            protected:
                void _writeVideo(
                    const std::string& fileName,
                    const otime::RationalTime&,
                    const std::shared_ptr<core::imaging::Image>&) override;

            private:
                int _quality = 90;
            };

            //! JPEG plugin.
            class Plugin : public IPlugin
            {
            protected:
                Plugin();

            public:
                //! Create a new plugin.
                static std::shared_ptr<Plugin> create(const std::weak_ptr<core::LogSystem>&);

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
