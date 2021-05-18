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
        struct VideoFrame
        {
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

            //! Get a video frame.
            virtual std::future<VideoFrame> getVideoFrame(const otime::RationalTime&) = 0;

            //! Cancel video frame requests.
            virtual void cancelVideoFrames() = 0;

        protected:
            otime::RationalTime _defaultSpeed = otime::RationalTime(0, 24);
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

            //! Can the plugin read the given file?
            virtual bool canRead(const std::string& fileName);

            //! Create a reader for the given file.
            virtual std::shared_ptr<IRead> read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed) = 0;

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

            // Can the given file be read?
            bool canRead(const std::string& fileName);

            // Create a reader for the given file.
            std::shared_ptr<IRead> read(
                const std::string& fileName,
                const otime::RationalTime& defaultSpeed = otime::RationalTime(0, 24));

        private:
            std::vector<std::shared_ptr<IPlugin> > _plugins;
        };
    }
}

#include <tlrCore/IOInline.h>
