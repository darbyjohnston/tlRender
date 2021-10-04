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

    //! Audio/visual I/O.
    namespace avio
    {
        //! Video type.
        enum class VideoType
        {
            Movie,
            Sequence
        };

        //! I/O information.
        struct Info
        {
            std::vector<imaging::Info>         video;
            VideoType                          videoType        = VideoType::Movie;
            otime::TimeRange                   videoTimeRange   = time::invalidTimeRange;
            audio::Info                        audio;
            size_t                             audioSampleCount = 0;
            std::map<std::string, std::string> tags;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Video I/O frame.
        struct VideoFrame
        {
            VideoFrame();
            VideoFrame(
                const otime::RationalTime&,
                uint16_t layer,
                const std::shared_ptr<imaging::Image>&);

            otime::RationalTime             time  = time::invalidTime;
            uint16_t                        layer = 0;
            std::shared_ptr<imaging::Image> image;

            bool operator == (const VideoFrame&) const;
            bool operator != (const VideoFrame&) const;
            bool operator < (const VideoFrame&) const;
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

            //! Read a video frame.
            virtual std::future<VideoFrame> readVideoFrame(
                const otime::RationalTime&,
                uint16_t layer = 0,
                const std::shared_ptr<imaging::Image>& = nullptr) = 0;

            //! Are there pending video frame requests?
            virtual bool hasVideoFrames() = 0;

            //! Cancel pending video frame requests.
            virtual void cancelVideoFrames() = 0;

            //! Stop ther reader.
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

            //! Write a video frame.
            virtual void writeVideoFrame(
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
                const std::set<std::string>& extensions,
                const std::shared_ptr<core::LogSystem>&);
            IPlugin();

        public:
            virtual ~IPlugin() = 0;

            //! Get the plugin name.
            const std::string& getName() const;

            //! Get the supported file extensions.
            const std::set<std::string>& getExtensions() const;

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
