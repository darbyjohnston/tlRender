// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>
#include <tlCore/FileIO.h>
#include <tlCore/Image.h>
#include <tlCore/Path.h>
#include <tlCore/Time.h>

#include <future>
#include <iostream>
#include <map>
#include <set>

namespace tl
{
    namespace log
    {
        class System;
    }

    //! Audio and video I/O.
    namespace io
    {
        //! File types.
        enum class FileType
        {
            Unknown  = 0,
            Movie    = 1,
            Sequence = 2,
            Audio    = 4,

            Count,
            First = Unknown
        };

        //! I/O information.
        struct Info
        {
            //! Video layer information.
            std::vector<imaging::Info> video;

            //! Video time range.
            otime::TimeRange videoTime = time::invalidTimeRange;

            //! Audio information.
            audio::Info audio;

            //! Audio time range.
            otime::TimeRange audioTime = time::invalidTimeRange;

            //! Metadata tags.
            imaging::Tags tags;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Video I/O data.
        struct VideoData
        {
            VideoData();
            VideoData(
                const otime::RationalTime&,
                uint16_t layer,
                const std::shared_ptr<imaging::Image>&);

            otime::RationalTime             time = time::invalidTime;
            uint16_t                        layer = 0;
            std::shared_ptr<imaging::Image> image;

            bool operator == (const VideoData&) const;
            bool operator != (const VideoData&) const;
            bool operator < (const VideoData&) const;
        };

        //! Audio I/O data.
        struct AudioData
        {
            AudioData();
            AudioData(
                const otime::RationalTime&,
                const std::shared_ptr<audio::Audio>&);

            otime::RationalTime           time = time::invalidTime;
            std::shared_ptr<audio::Audio> audio;

            bool operator == (const AudioData&) const;
            bool operator != (const AudioData&) const;
            bool operator < (const AudioData&) const;
        };

        //! Options.
        typedef std::map<std::string, std::string> Options;

        //! Merge options.
        Options merge(const Options&, const Options&);

        //! Base class for readers/writers.
        class IIO : public std::enable_shared_from_this<IIO>
        {
            TLRENDER_NON_COPYABLE(IIO);

        protected:
            void _init(
                const file::Path&,
                const Options&,
                const std::weak_ptr<log::System>&);

            IIO();

        public:
            virtual ~IIO() = 0;

            //! Get the path.
            const file::Path& getPath() const;

        protected:
            std::weak_ptr<log::System> _logSystem;
            file::Path _path;
            Options _options;
        };

        //! Base class for readers.
        class IRead : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const Options&,
                const std::weak_ptr<log::System>&);

            IRead();

        public:
            ~IRead() override;

            //! Get the information.
            virtual std::future<Info> getInfo() = 0;

            //! Read video data.
            virtual std::future<VideoData> readVideo(const otime::RationalTime&, uint16_t layer = 0);

            //! Read audio data.
            virtual std::future<AudioData> readAudio(const otime::TimeRange&);

            //! Cancel pending requests.
            virtual void cancelRequests() = 0;

            //! Stop the reader.
            virtual void stop() = 0;

        protected:
            std::vector<file::MemoryRead> _memory;
        };

        //! Base class for writers.
        class IWrite : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const Options&,
                const Info&,
                const std::weak_ptr<log::System>&);

            IWrite();

        public:
            ~IWrite() override;

            //! Write video data.
            virtual void writeVideo(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) = 0;

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
                const std::weak_ptr<log::System>&);

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

            //! Set the plugin options.
            void setOptions(const Options&);

            //! Create a reader for the given path.
            virtual std::shared_ptr<IRead> read(
                const file::Path&,
                const Options & = Options()) = 0;

            //! Create a reader for the given path and memory locations.
            virtual std::shared_ptr<IRead> read(
                const file::Path&,
                const std::vector<file::MemoryRead>&,
                const Options & = Options()) = 0;

            //! Get information for writing.
            virtual imaging::Info getWriteInfo(
                const imaging::Info&,
                const Options& = Options()) const = 0;

            //! Create a writer for the given path.
            virtual std::shared_ptr<IWrite> write(
                const file::Path&,
                const Info&,
                const Options& = Options()) = 0;

        protected:
            bool _isWriteCompatible(const imaging::Info&, const Options&) const;

            std::weak_ptr<log::System> _logSystem;
            Options _options;

        private:
            TLRENDER_PRIVATE();
        };
    }
}

#include <tlIO/IOInline.h>
