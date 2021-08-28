// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/AVIO.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

} // extern "C"

namespace tlr
{
    //! FFmpeg I/O
    namespace ffmpeg
    {
        //! Profiles.
        enum class Profile
        {
            H264,
            ProRes,
            ProRes_Proxy,
            ProRes_LT,
            ProRes_HQ,
            ProRes_4444,
            ProRes_XQ,

            Count
        };
        TLR_ENUM(Profile);
        TLR_ENUM_SERIALIZE(Profile);

        //! Number of threads.
        const size_t threadCount = 8;

        //! Timeout for frame requests.
        const std::chrono::microseconds requestTimeout(1000);

        //! Software scaler flags.
        const int swsScaleFlags = SWS_FAST_BILINEAR;

        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! Swap the numerator and denominator.
        AVRational swap(AVRational);

        //! FFmpeg reader
        class Read : public avio::IRead
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);
            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);

            std::future<avio::Info> getInfo() override;
            std::future<avio::VideoFrame> readVideoFrame(const otime::RationalTime&) override;
            bool hasVideoFrames() override;
            void cancelVideoFrames() override;
            void stop() override;
            bool hasStopped() const override;

        private:
            void _open(const std::string& fileName);
            void _run();
            void _close();

            TLR_PRIVATE();
        };

        //! FFmpeg writer.
        class Write : public avio::IWrite
        {
        protected:
            void _init(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);
            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const avio::Info&,
                const avio::Options&,
                const std::shared_ptr<core::LogSystem>&);

            void writeVideoFrame(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;

        private:
            void _encodeVideo(AVFrame*);

            TLR_PRIVATE();
        };

        //! FFmpeg Plugin
        class Plugin : public avio::IPlugin
        {
        protected:
            void _init(const std::shared_ptr<core::LogSystem>&);
            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::shared_ptr<core::LogSystem>&);

            std::shared_ptr<avio::IRead> read(
                const file::Path&,
                const avio::Options& = avio::Options()) override;
            std::vector<imaging::PixelType> getWritePixelTypes() const override;
            std::shared_ptr<avio::IWrite> write(
                const file::Path&,
                const avio::Info&,
                const avio::Options& = avio::Options()) override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            static std::weak_ptr<core::LogSystem> _logSystemWeak;
        };
    }
}
