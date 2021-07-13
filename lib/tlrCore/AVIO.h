// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Image.h>
#include <tlrCore/Time.h>

#include <future>
#include <iostream>
#include <map>
#include <set>

namespace tlr
{
    //! Input/output.
    namespace avio
    {
        //! I/O information.
        struct Info
        {
            Info();

            std::vector<imaging::Info>         video;
            otime::RationalTime                videoDuration;
            std::map<std::string, std::string> tags;
        };

        //! Video I/O frame.
        class VideoFrame
        {
        public:
            VideoFrame();
            VideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&);

            otime::RationalTime             time;
            std::shared_ptr<imaging::Image> image;

            bool operator == (const VideoFrame&) const;
            bool operator != (const VideoFrame&) const;
            bool operator < (const VideoFrame&) const;
        };

        //! Options.
        typedef std::map<std::string, std::string> Options;

        //! Base class for readers/writers.
        class IIO : public std::enable_shared_from_this<IIO>
        {
            TLR_NON_COPYABLE(IIO);

        protected:
            void _init(
                const std::string& fileName,
                const Options&);
            IIO();

        public:
            virtual ~IIO() = 0;
            
            //! Get the file name.
            const std::string& getFileName() const;

        protected:
            std::string _fileName;
            Options _options;
        };

        //! Base class for readers.
        class IRead : public IIO
        {
        protected:
            void _init(
                const std::string& fileName,
                const Options&);
            IRead();

        public:
            ~IRead() override;

            //! Get the information.
            virtual std::future<Info> getInfo() = 0;

            //! Read a video frame.
            virtual std::future<VideoFrame> readVideoFrame(const otime::RationalTime&) = 0;

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
                const std::string& fileName,
                const Options&,
                const Info&);
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
                const std::set<std::string>& extensions);
            IPlugin();

        public:
            virtual ~IPlugin() = 0;

            //! Get the plugin name.
            const std::string& getName() const;

            //! Get the supported file extensions.
            const std::set<std::string>& getExtensions() const;

            //! Create a reader for the given file.
            virtual std::shared_ptr<IRead> read(
                const std::string& fileName,
                const Options& = Options()) = 0;

            //! Get the list of writable image pixel types.
            virtual std::vector<imaging::PixelType> getWritePixelTypes() const = 0;

            //! Get the writable image data alignment.
            virtual uint8_t getWriteAlignment(imaging::PixelType) const;

            //! Get the writable image data endian.
            virtual memory::Endian getWriteEndian() const;

            //! Create a writer for the given file.
            virtual std::shared_ptr<IWrite> write(
                const std::string& fileName,
                const Info&,
                const Options& = Options()) = 0;

        protected:
            bool _isWriteCompatible(const imaging::Info&) const;

        private:
            TLR_PRIVATE();
        };

        //! I/O system.
        class System : public std::enable_shared_from_this<System>
        {
            TLR_NON_COPYABLE(System);

        protected:
            void _init();
            System();

        public:
            ~System();

            //! Create a new I/O system.
            static std::shared_ptr<System> create();

            //! Get the list of plugins.
            const std::vector<std::shared_ptr<IPlugin> >& getPlugins() const;

            //! Get a plugin for the given file name.
            std::shared_ptr<IPlugin> getPlugin(const std::string& fileName) const;

            // Create a reader for the given file name.
            std::shared_ptr<IRead> read(
                const std::string& fileName,
                const Options& = Options());

            // Create a writer for the given file name.
            std::shared_ptr<IWrite> write(
                const std::string& fileName,
                const Info&,
                const Options& = Options());

        private:
            TLR_PRIVATE();
        };
    }
}

#include <tlrCore/AVIOInline.h>
