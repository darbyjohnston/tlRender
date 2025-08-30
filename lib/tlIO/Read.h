// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Plugin.h>

namespace tl
{
    namespace io
    {
        //! Base class for readers.
        class IRead : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<ftk::InMemoryFile>&,
                const Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            IRead();

        public:
            virtual ~IRead();

            //! Get the information.
            virtual std::future<Info> getInfo() = 0;

            //! Read video data.
            virtual std::future<VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const Options& = Options());

            //! Read audio data.
            virtual std::future<AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const Options& = Options());

            //! Cancel pending requests.
            virtual void cancelRequests() = 0;

        protected:
            std::vector<ftk::InMemoryFile> _memory;
        };

        //! Base class for read plugins.
        class IReadPlugin : public IPlugin
        {
            FTK_NON_COPYABLE(IReadPlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<ftk::LogSystem>&);

            IReadPlugin();

        public:
            virtual ~IReadPlugin() = 0;

            //! Create a reader for the given path.
            virtual std::shared_ptr<IRead> read(
                const file::Path&,
                const Options& = Options()) = 0;

            //! Create a reader for the given path and memory locations.
            virtual std::shared_ptr<IRead> read(
                const file::Path&,
                const std::vector<ftk::InMemoryFile>&,
                const Options& = Options()) = 0;

        private:
            FTK_PRIVATE();
        };
    }
}
