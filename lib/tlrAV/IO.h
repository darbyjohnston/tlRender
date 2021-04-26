// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrAV/Image.h>

#include <tlrTimeline/Util.h>

#include <iostream>
#include <map>
#include <queue>

namespace tlr
{
    //! Audio/Video
    namespace av
    {
        //! I/O
        namespace io
        {
            //! Video Information
            struct VideoInfo
            {
                imaging::Info info;
                opentime::RationalTime duration;
                std::string codec;
            };

            //! I/O Information
            struct Info
            {
                std::vector<VideoInfo> video;
                std::map<std::string, std::string> tags;
            };

            //! Video Frame
            struct VideoFrame
            {
                opentime::RationalTime time;
                std::shared_ptr<imaging::Image> image;
            };

            //! I/O Interface
            class IIO : std::enable_shared_from_this<IIO>
            {
                TLR_NON_COPYABLE(IIO);

            protected:
                void _init(const std::string& fileName);
                IIO();

            public:
                virtual ~IIO() = 0;

                //! Get the file name.
                const std::string& getFileName() const;

                //! Get the I/O information.
                const Info& getInfo() const;

            protected:
                std::string _fileName;
                Info _info;
            };

            //! Reader Interface
            class IRead : public IIO
            {
            protected:
                void _init(
                    const std::string& fileName,
                    size_t videoQueueSize);
                IRead();

            public:
                //! Seek to the given time.
                void seek(const opentime::RationalTime&);

                //! This function should be called periodically to let the plugin do work.
                virtual void tick() = 0;

                //! Get the queue of video frames.
                std::queue<VideoFrame>& getVideoQueue();

            protected:
                bool _hasSeek = false;
                opentime::RationalTime _seekTime;
                std::queue<VideoFrame> _videoQueue;
                size_t _videoQueueSize = 0;
            };

            //! I/O Plugin Interface
            class IPlugin : std::enable_shared_from_this<IIO>
            {
                TLR_NON_COPYABLE(IPlugin);

            protected:
                void _init();
                IPlugin();

            public:
                virtual ~IPlugin() = 0;

                //! Can the plugin read the given file?
                virtual bool canRead(const std::string& fileName) = 0;

                //! Create a reader for the given file.
                virtual std::shared_ptr<IRead> read(const std::string& fileName) = 0;

                // Set the video queue size.
                void setVideoQueueSize(size_t);

            protected:
                size_t _videoQueueSize = 0;
            };

            //! I/O System
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
                std::shared_ptr<IRead> read(const std::string& fileName);

                // Set the video queue size.
                void setVideoQueueSize(size_t);

            private:
                std::vector<std::shared_ptr<IPlugin> > _plugins;
                size_t _videoQueueSize = 10;
            };
        }
    }
}

#include <tlrAV/IOInline.h>
