// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Audio.h>
#include <tlrCore/Image.h>
#include <tlrCore/Path.h>
#include <tlrCore/Time.h>

#include <future>
#include <iostream>
#include <map>
#include <set>

namespace tlr
{
    namespace core
    {
        class LogSystem;
    }

    //! Audio/video I/O.
    namespace avio
    {
        //! Video type.
        enum class VideoType
        {
            Movie,
            Sequence,

            Count,
            First = Movie
        };
        TLR_ENUM(VideoType);
        TLR_ENUM_SERIALIZE(VideoType);

        //! File extension types.
        enum class FileExtensionType
        {
            VideoAndAudio = 1,
            VideoOnly     = 2,
            AudioOnly     = 4,

            Count,
            First = VideoAndAudio
        };
        TLR_ENUM(FileExtensionType);
        TLR_ENUM_SERIALIZE(FileExtensionType);

        //! I/O information.
        struct Info
        {
            std::vector<imaging::Info>         video;
            VideoType                          videoType = VideoType::Movie;
            otime::TimeRange                   videoTime = time::invalidTimeRange;
            audio::Info                        audio;
            otime::TimeRange                   audioTime = time::invalidTimeRange;
            std::map<std::string, std::string> tags;

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

            otime::RationalTime             time  = time::invalidTime;
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

            otime::RationalTime           time  = time::invalidTime;
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
            TLR_NON_COPYABLE(IIO);

        protected:
            void _init(
                const file::Path&,
                const Options&,
                const std::shared_ptr<core::LogSystem>&);
            IIO();

        public:
            virtual ~IIO() = 0;
            
            //! Get the path.
            const file::Path& getPath() const;

        protected:
            std::shared_ptr<core::LogSystem> _logSystem;
            file::Path _path;
            Options _options;
        };

        //! Base class for readers.
        class IRead : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const Options&,
                const std::shared_ptr<core::LogSystem>&);
            IRead();

        public:
            ~IRead() override;

            //! Get the information.
            virtual std::future<Info> getInfo() = 0;

            //! Read video data.
            virtual std::future<VideoData> readVideo(const otime::RationalTime&, uint16_t layer = 0);

            //! Read audio data.
            virtual std::future<AudioData> readAudio(const otime::TimeRange&);

            //! Are there pending requests?
            virtual bool hasRequests() = 0;

            //! Cancel pending requests.
            virtual void cancelRequests() = 0;

            //! Stop the reader.
            virtual void stop() = 0;

            //! Has the reader stopped?
            virtual bool hasStopped() const = 0;
        };
        
        //! Base class for writers.
        class IWrite : public IIO
        {
        protected:
            void _init(
                const file::Path&,
                const Options&,
                const Info&,
                const std::shared_ptr<core::LogSystem>&);
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
            TLR_NON_COPYABLE(IPlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileExtensionType>& extensions,
                const std::shared_ptr<core::LogSystem>&);
            IPlugin();

        public:
            virtual ~IPlugin() = 0;

            //! Get the plugin name.
            const std::string& getName() const;

            //! Get the supported file extensions.
            std::set<std::string> getExtensions(
                int types = static_cast<int>(FileExtensionType::VideoAndAudio) |
                static_cast<int>(FileExtensionType::VideoOnly) |
                static_cast<int>(FileExtensionType::AudioOnly)) const;

            //! Set the plugin options.
            void setOptions(const Options&);

            //! Create a reader for the given path.
            virtual std::shared_ptr<IRead> read(
                const file::Path&,
                const Options& = Options()) = 0;

            //! Get the list of writable image pixel types.
            virtual std::vector<imaging::PixelType> getWritePixelTypes() const = 0;

            //! Get the writable image data alignment.
            virtual uint8_t getWriteAlignment(imaging::PixelType) const;

            //! Get the writable image data endian.
            virtual memory::Endian getWriteEndian() const;

            //! Create a writer for the given path.
            virtual std::shared_ptr<IWrite> write(
                const file::Path&,
                const Info&,
                const Options & = Options()) = 0;

        protected:
            bool _isWriteCompatible(const imaging::Info&) const;

            std::shared_ptr<core::LogSystem> _logSystem;
            Options _options;

        private:
            TLR_PRIVATE();
        };
    }
}

#include <tlrCore/AVIOInline.h>
