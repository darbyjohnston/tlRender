// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Read.h>
#include <tlIO/Write.h>

#include <tlCore/HDR.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace tl
{
    //! FFmpeg video and audio I/O
    namespace ffmpeg
    {
        //! Write profiles.
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
        DTK_ENUM(Profile);

        //! FFmpeg options.
        struct Options
        {
            bool yuvToRgb = false;
            size_t threadCount = 0;

            bool operator == (const Options&) const;
            bool operator != (const Options&) const;
        };

        //! Get FFmpeg options.
        io::Options getOptions(const Options&);

        //! Software scaler flags.
        const int swsScaleFlags = SWS_FAST_BILINEAR;

        //! Swap the numerator and denominator.
        AVRational swap(AVRational);

        //! Convert to HDR data.
        void toHDRData(AVFrameSideData**, int size, image::HDRData&);

        //! Convert from FFmpeg.
        audio::DataType toAudioType(AVSampleFormat);

        //! Convert to FFmpeg.
        AVSampleFormat fromAudioType(audio::DataType);

        //! Get the timecode from a data stream if it exists.
        std::string getTimecodeFromDataStream(AVFormatContext*);

        //! RAII class for FFmpeg packets.
        class Packet
        {
        public:
            Packet();
            ~Packet();

            AVPacket* p = nullptr;
        };

        //! Get a label for a FFmpeg error code.
        std::string getErrorLabel(int);

        //! FFmpeg reader
        class Read : public io::IRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            std::future<io::Info> getInfo() override;
            std::future<io::VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const io::Options& = io::Options()) override;
            std::future<io::AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const io::Options& = io::Options()) override;
            void cancelRequests() override;

        private:
            void _videoThread();
            void _audioThread();
            void _cancelVideoRequests();
            void _cancelAudioRequests();

            DTK_PRIVATE();
        };

        //! FFmpeg writer.
        class Write : public io::IWrite
        {
        protected:
            void _init(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<dtk::LogSystem>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<dtk::LogSystem>&);

            void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<dtk::Image>&,
                const io::Options& = io::Options()) override;

        private:
            void _encodeVideo(AVFrame*);

            DTK_PRIVATE();
        };

        //! FFmpeg read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            ReadPlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<io::Cache>&,
                const std::shared_ptr<dtk::LogSystem>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<dtk::InMemoryFile>&,
                const io::Options & = io::Options()) override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            //! \todo What is a better way to access the log system from the
            //! FFmpeg callback?
            static std::weak_ptr<dtk::LogSystem> _logSystemWeak;
        };

        //! FFmpeg write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<dtk::LogSystem>&);

            WritePlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<dtk::LogSystem>&);

            //! Get the list of codecs.
            std::vector<std::string> getCodecs() const;

            dtk::ImageInfo getInfo(
                const dtk::ImageInfo&,
                const io::Options & = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options & = io::Options()) override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            //! \todo What is a better way to access the log system from the
            //! FFmpeg callback?
            static std::weak_ptr<dtk::LogSystem> _logSystemWeak;
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Options&);

        void from_json(const nlohmann::json&, Options&);

        ///@}
    }
}
