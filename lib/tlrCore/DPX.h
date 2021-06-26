// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/SequenceIO.h>

namespace tlr
{
    //! DPX I/O.
    namespace dpx
    {
        //! DPX reader.
        class Read : public avio::ISequenceRead
        {
        protected:
            void _init(
                const std::string& fileName,
                const avio::Options&);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const std::string& fileName,
                const avio::Options&);

        protected:
            avio::Info _getInfo(const std::string& fileName) override;
            avio::VideoFrame _readVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&) override;
        };

        //! DPX writer.
        class Write : public avio::ISequenceWrite
        {
        protected:
            void _init(
                const std::string& fileName,
                const avio::Info&,
                const avio::Options&);
            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const std::string& fileName,
                const avio::Info&,
                const avio::Options&);

        protected:
            void _writeVideoFrame(
                const std::string& fileName,
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;
        };

        //! DPX plugin.
        class Plugin : public avio::IPlugin
        {
        protected:
            void _init();
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create();

            std::shared_ptr<avio::IRead> read(
                const std::string& fileName,
                const avio::Options& = avio::Options()) override;
            std::vector<imaging::PixelType> getWritePixelTypes() const override;
            std::shared_ptr<avio::IWrite> write(
                const std::string& fileName,
                const avio::Info&,
                const avio::Options& = avio::Options()) override;
        };
    }
}
