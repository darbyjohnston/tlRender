// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/Read.h>
#include <tlIO/Write.h>

struct AVFrame;

namespace tl
{
    //! FFmpeg video and audio I/O
    namespace ffmpeg
    {
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

        //! FFmpeg reader
        class Read : public io::IRead
        {
        protected:
            void _init(
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

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

            FEATHER_TK_PRIVATE();
        };

        //! FFmpeg writer.
        class Write : public io::IWrite
        {
        protected:
            void _init(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
                const file::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<feather_tk::LogSystem>&);

            void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<feather_tk::Image>&,
                const io::Options& = io::Options()) override;

        private:
            void _encodeVideo(AVFrame*);

            FEATHER_TK_PRIVATE();
        };

        //! FFmpeg read plugin.
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<feather_tk::LogSystem>&);

            ReadPlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<feather_tk::LogSystem>&);

            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const io::Options & = io::Options()) override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            //! \todo What is a better way to access the log system from the
            //! FFmpeg callback?
            static std::weak_ptr<feather_tk::LogSystem> _logSystemWeak;

            FEATHER_TK_PRIVATE();
        };

        //! FFmpeg write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<feather_tk::LogSystem>&);

            WritePlugin();

        public:
            //! Create a new plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<feather_tk::LogSystem>&);

            //! Get the list of codecs.
            const std::vector<std::string>& getCodecs() const;

            feather_tk::ImageInfo getInfo(
                const feather_tk::ImageInfo&,
                const io::Options & = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const file::Path&,
                const io::Info&,
                const io::Options & = io::Options()) override;

        private:
            static void _logCallback(void*, int, const char*, va_list);

            //! \todo What is a better way to access the log system from the
            //! FFmpeg callback?
            static std::weak_ptr<feather_tk::LogSystem> _logSystemWeak;

            FEATHER_TK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const Options&);

        void from_json(const nlohmann::json&, Options&);

        ///@}
    }
}
