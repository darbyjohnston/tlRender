// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Read.h>

namespace tl
{
    namespace wmf
    {
        //! WMF options.
        struct Options
        {
            bool operator == (const Options&) const;
            bool operator != (const Options&) const;
        };

        //! Get WMF options.
        io::Options getOptions(const Options&);

        //! WMF reader
        class Read : public io::IRead
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

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const io::Options& = io::Options()) override;
            std::future<io::AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const io::Options& = io::Options()) override;
            void cancelRequests() override;

        private:
            void _thread();

            FTK_PRIVATE();
        };

        //! WMF read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

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
                const io::Options& = io::Options()) override;

        private:
            FTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Options&);

        void from_json(const nlohmann::json&, Options&);

        ///@}
    }
}
