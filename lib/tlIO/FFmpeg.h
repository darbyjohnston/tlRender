// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/HDR.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

} // extern "C"

namespace tl
{
    //! FFmpeg I/O
    namespace ffmpeg
    {
        //! Profiles.
        enum class Profile
        {
            None,
            H264,
            ProRes,
            ProRes_Proxy,
            ProRes_LT,
            ProRes_HQ,
            ProRes_4444,
            ProRes_XQ,

            Count
        };
        TLRENDER_ENUM(Profile);
        TLRENDER_ENUM_SERIALIZE(Profile);

        //! Number of threads.
        const size_t threadCount = 0;

        //! Timeout for requests.
        const std::chrono::milliseconds requestTimeout(5);

        //! Software scaler flags.
        const int swsScaleFlags = SWS_FAST_BILINEAR;

        //! Swap the numerator and denominator.
        AVRational swap(AVRational);

        //! Convert to HDR data.
        void toHDRData(AVFrameSideData**, int size, imaging::HDRData&);

        //! Convert to FFmpeg channel layout.
        int64_t fromChannelCount(uint8_t);

        //! Convert from FFmpeg.
        audio::DataType toAudioType(AVSampleFormat);

        //! Convert to FFmpeg.
        AVSampleFormat fromAudioType(audio::DataType);

        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! FFmpeg reader
        class Read : public io::IRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<io::MemoryRead>&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            Read();

        public:
            ~Read() override;

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const std::vector<io::MemoryRead>&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(const otime::RationalTime&, uint16_t layer = 0) override;
            std::future<io::AudioData> readAudio(const otime::TimeRange&) override;
            bool hasRequests() override;
            void cancelRequests() override;
            void stop() override;
            bool hasStopped() const override;

        private:
            void _openVideo(const std::string& fileName);
            void _openAudio(const std::string& fileName);
            void _run();
            void _close();

            TLRENDER_PRIVATE();
        };

        //! FFmpeg writer.
        class Write : public io::IWrite
        {
        protected:
            void _init(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            Write();

        public:
            ~Write() override;

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::weak_ptr<log::System>&);

            void writeVideo(
                const otime::RationalTime&,
                const std::shared_ptr<imaging::Image>&) override;

        private:
            void _encodeVideo(AVFrame*);

            TLRENDER_PRIVATE();
        };

        //! FFmpeg Plugin
        class Plugin : public io::IPlugin
        {
        protected:
            void _init(const std::weak_ptr<log::System>&);

            Plugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<Plugin> create(const std::weak_ptr<log::System>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<io::MemoryRead>&,
                const io::Options & = io::Options()) override;
            imaging::Info getWriteInfo(
                const imaging::Info&,
                const io::Options& = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options& = io::Options()) override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            //! \todo What is a better way to access the log system from the
            //! FFmpeg callback?
            static std::weak_ptr<log::System> _logSystemWeak;
        };
    }
}
