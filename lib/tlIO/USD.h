// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

namespace tl
{
    namespace usd
    {
        class Render;
        
        //! USD reader.
        class Read : public io::IRead
        {
        protected:
            void _init(
                int64_t id,
                const std::shared_ptr<Render>&,
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                int64_t id,
                const std::shared_ptr<Render>&,
                const file::Path&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(const otime::RationalTime&, uint16_t layer = 0) override;
            void cancelRequests() override;

        private:
            TLRENDER_PRIVATE();
        };

        //! USD plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(const std::weak_ptr<log::System>&);
            
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::weak_ptr<log::System>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const io::Options & = io::Options()) override;
            imaging::Info getWriteInfo(
                const imaging::Info&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;
                
        private:
            TLRENDER_PRIVATE();
        };
    }
}

