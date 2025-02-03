// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Cache.h>

#include <dtk/core/FileIO.h>

#include <future>
#include <set>

namespace dtk
{
    class LogSystem;
}

namespace tl
{
    namespace io
    {
        //! Options.
        typedef std::map<std::string, std::string> Options;

        //! Merge options.
        Options merge(const Options&, const Options&);

        //! Base class for readers and writers.
        class IIO : public std::enable_shared_from_this<IIO>
        {
            TLRENDER_NON_COPYABLE(IIO);

        protected:
            void _init(
                const file::Path&,
                const Options&,
                const std::shared_ptr<Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            IIO();

        public:
            virtual ~IIO() = 0;

            //! Get the path.
            const file::Path& getPath() const;

        protected:
            file::Path _path;
            Options _options;
            std::shared_ptr<Cache> _cache;
            std::weak_ptr<dtk::LogSystem> _logSystem;
        };

        //! Base class for readers.
        class IRead : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const Options&,
                const std::shared_ptr<Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

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
            std::vector<dtk::InMemoryFile> _memory;
        };

        //! Base class for writers.
        class IWrite : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const Options&,
                const Info&,
                const std::shared_ptr<dtk::LogSystem>&);

            IWrite();

        public:
            virtual ~IWrite();

            //! Write video data.
            virtual void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<image::Image>&,
                const Options& = Options()) = 0;

        protected:
            Info _info;
        };

        //! Base class for I/O plugins.
        class IPlugin : public std::enable_shared_from_this<IPlugin>
        {
            TLRENDER_NON_COPYABLE(IPlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            IPlugin();

        public:
            virtual ~IPlugin() = 0;

            //! Get the plugin name.
            const std::string& getName() const;

            //! Get the supported file extensions.
            std::set<std::string> getExtensions(int types =
                static_cast<int>(FileType::Movie) |
                static_cast<int>(FileType::Sequence) |
                static_cast<int>(FileType::Audio)) const;

            //! Create a reader for the given path.
            virtual std::shared_ptr<IRead> read(
                const file::Path&,
                const Options& = Options()) = 0;

            //! Create a reader for the given path and memory locations.
            virtual std::shared_ptr<IRead> read(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const Options& = Options()) = 0;

            //! Get information for writing.
            virtual image::Info getWriteInfo(
                const image::Info&,
                const Options& = Options()) const = 0;

            //! Create a writer for the given path.
            virtual std::shared_ptr<IWrite> write(
                const file::Path&,
                const Info&,
                const Options& = Options()) = 0;

        protected:
            bool _isWriteCompatible(const image::Info&, const Options&) const;

            std::shared_ptr<Cache> _cache;
            std::weak_ptr<dtk::LogSystem> _logSystem;

        private:
            TLRENDER_PRIVATE();
        };
    }
}

#include <tlIO/PluginInline.h>
