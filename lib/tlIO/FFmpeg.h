// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

} // extern "C"

namespace tl
{
    namespace io
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
            TLRENDER_ENUM(Profile);
            TLRENDER_ENUM_SERIALIZE(Profile);

            //! Number of threads.
            const size_t threadCount = 4;

            //! Timeout for requests.
            const std::chrono::milliseconds requestTimeout(1);

            //! Software scaler flags.
            const int swsScaleFlags = SWS_FAST_BILINEAR;

            //! Swap the numerator and denominator.
            AVRational swap(AVRational);

            //! Convert to FFmpeg channel layout.
            int64_t fromChannelCount(uint8_t);

            //! Convert from FFmpeg.
            core::audio::DataType toAudioType(AVSampleFormat);

            //! Convert to FFmpeg.
            AVSampleFormat fromAudioType(core::audio::DataType);

            //! Get a label for a FFmpeg error code.
            std::string getErrorLabel(int);

            //! FFmpeg reader
            class Read : public IRead
            {
            protected:
                void _init(
                    const core::file::Path&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);
                Read();

            public:
                ~Read() override;

                //! Create a new reader.
                static std::shared_ptr<Read> create(
                    const core::file::Path&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);

                std::future<Info> getInfo() override;
                std::future<VideoData> readVideo(const otime::RationalTime&, uint16_t layer = 0) override;
                std::future<AudioData> readAudio(const otime::TimeRange&) override;
                bool hasRequests() override;
                void cancelRequests() override;
                void stop() override;
                bool hasStopped() const override;

            private:
                void _open(const std::string& fileName);
                void _run();
                void _close();

                TLRENDER_PRIVATE();
            };

            //! FFmpeg writer.
            class Write : public IWrite
            {
            protected:
                void _init(
                    const core::file::Path&,
                    const Info&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);
                Write();

            public:
                ~Write() override;

                //! Create a new writer.
                static std::shared_ptr<Write> create(
                    const core::file::Path&,
                    const Info&,
                    const Options&,
                    const std::weak_ptr<core::log::System>&);

                void writeVideo(
                    const otime::RationalTime&,
                    const std::shared_ptr<core::imaging::Image>&) override;

            private:
                void _encodeVideo(AVFrame*);

                TLRENDER_PRIVATE();
            };

            //! FFmpeg Plugin
            class Plugin : public IPlugin
            {
            protected:
                void _init(const std::weak_ptr<core::log::System>&);
                Plugin();

            public:
                //! Create a new plugin.
                static std::shared_ptr<Plugin> create(const std::weak_ptr<core::log::System>&);

                std::shared_ptr<IRead> read(
                    const core::file::Path&,
                    const Options & = Options()) override;
                core::imaging::Info getWriteInfo(
                    const core::imaging::Info&,
                    const Options & = Options()) const override;
                std::shared_ptr<IWrite> write(
                    const core::file::Path&,
                    const Info&,
                    const Options & = Options()) override;

            private:
                static void _logCallback(void*, int, const char*, va_list);

                static std::weak_ptr<core::log::System> _logSystemWeak;
            };
        }
    }
}
