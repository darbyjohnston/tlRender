// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Image.h>

#include <future>
#include <iostream>
#include <map>
#include <set>

namespace tlr
{
    //! Input/output.
    namespace io
    {
        //! Video I/O information.
        struct VideoInfo
        {
            imaging::Info info;
            otime::RationalTime duration;
            std::string codec;
        };

        //! I/O information.
        struct Info
        {
            std::vector<VideoInfo> video;
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

            otime::RationalTime time;
            std::shared_ptr<imaging::Image> image;

            bool operator == (const VideoFrame&) const;
            bool operator != (const VideoFrame&) const;
            bool operator < (const VideoFrame&) const;
        };

        //! Base class for readers/writers.
        class IIO : public std::enable_shared_from_this<IIO>
        {
            TLR_NON_COPYABLE(IIO);

        protected:
            void _init(const std::string& fileName);
            IIO();

        public:
            virtual ~IIO() = 0;
            
            //! Get the file name.
            const std::string& getFileName() const;

        protected:
            std::string _fileName;
        };

        //! Base class for readers.
        class IRead : public IIO
        {
        protected:
            void _init(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed);
            IRead();

        public:
            ~IRead() override;

            //! Get the information.
            virtual std::future<Info> getInfo() = 0;

            //! Read a video frame.
            virtual std::future<VideoFrame> readVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>& = nullptr) = 0;

            //! Are there pending video frame requests?
            virtual bool hasVideoFrames() = 0;

            //! Cancel pending video frame requests.
            virtual void cancelVideoFrames() = 0;

        protected:
            otime::RationalTime _defaultSpeed = otime::RationalTime(0, 24);
        };
        
        //! Base class for writers.
        class IWrite : public IIO
        {
        protected:
            void _init(
                const std::string& fileName,
                const io::Info&);
            IWrite();

        public:
            ~IWrite() override;

            //! Write a video frame.
            virtual void writeVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) = 0;

        protected:
            io::Info _info;
        };

        //! Base class for I/O plugins.
        class IPlugin : public std::enable_shared_from_this<IPlugin>
        {
            TLR_NON_COPYABLE(IPlugin);

        protected:
            void _init(const std::set<std::string>&);
            IPlugin();

        public:
            virtual ~IPlugin() = 0;

            //! Get the supported file extensions.
            const std::set<std::string>& getExtensions() const;

            //! Create a reader for the given file.
            virtual std::shared_ptr<IRead> read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed) = 0;

            //! Get the list of writable pixel types.
            virtual std::vector<imaging::PixelType> getWritePixelTypes() const = 0;

            //! Create a writer for the given file.
            virtual std::shared_ptr<IWrite> write(
                const std::string& fileName,
                const io::Info&) = 0;

        private:
            std::set<std::string> _extensions;
        };

        //! I/O system.
        class System : public std::enable_shared_from_this<System>
        {
            TLR_NON_COPYABLE(System);

        protected:
            void _init();
            System();

        public:
            //! Create a new I/O system.
            static std::shared_ptr<System> create();

            // Create a reader for the given file.
            std::shared_ptr<IRead> read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed = otime::RationalTime(0, 24));

            // Create a writer for the given file.
            std::shared_ptr<IWrite> write(
                const std::string& fileName,
                const io::Info&);

        private:
            std::vector<std::shared_ptr<IPlugin> > _plugins;
        };
    }
}

#include <tlrCore/IOInline.h>
