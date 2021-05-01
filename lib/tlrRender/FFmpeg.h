// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrRender/IO.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

} // extern "C"

#include <map>

namespace tlr
{
    //! FFmpeg I/O
    namespace ffmpeg
    {
        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! FFmpeg Reader
        class Read : public io::IRead
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
            int _decodeVideo(AVPacket&, io::VideoFrame&);

            AVFormatContext* _avFormatContext = nullptr;
            int _avVideoStream = -1;
            std::map<int, AVCodecParameters*> _avCodecParameters;
            std::map<int, AVCodecContext*> _avCodecContext;
            AVFrame* _avFrame = nullptr;
            AVFrame* _avFrameRgb = nullptr;
            SwsContext* _swsContext = nullptr;
        };

        //! FFmpeg Plugin
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
