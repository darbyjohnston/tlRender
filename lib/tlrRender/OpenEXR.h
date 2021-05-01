// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrRender/IO.h>

namespace tlr
{
    //! OpenEXR I/O.
    namespace exr
    {
        //! OpenEXR reader.
        class Read : public io::ISequenceRead
        {
        protected:
            void _init(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed,
                size_t videoQueueSize);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed,
                size_t videoQueueSize);
                
            void tick() override;

        private:
        };

        //! OpenEXR plugin.
        class Plugin : public io::IPlugin
        {
        protected:
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create();

            bool canRead(const std::string&) override;
            std::shared_ptr<io::IRead> read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed) override;
        };
    }
}
