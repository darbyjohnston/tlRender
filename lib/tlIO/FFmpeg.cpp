// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpeg.h>

#include <dtk/core/Assert.h>
#include <dtk/core/Format.h>
#include <dtk/core/LogSystem.h>

extern "C"
{
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/hdr_dynamic_metadata.h>
#include <libavutil/imgutils.h>
#include <libavutil/mastering_display_metadata.h>
}

namespace tl
{
    namespace ffmpeg
    {
        bool Options::operator == (const Options& other) const
        {
            return
                yuvToRgb == other.yuvToRgb &&
                threadCount == other.threadCount;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        io::Options getOptions(const Options& value)
        {
            io::Options out;
            out["FFmpeg/YUVToRGB"] = dtk::Format("{0}").arg(value.yuvToRgb);
            out["FFmpeg/ThreadCount"] = dtk::Format("{0}").arg(value.threadCount);
            return out;
        }

        AVRational swap(AVRational value)
        {
            return AVRational({ value.den, value.num });
        }

        void toHDRData(AVFrameSideData** sideData, int size, image::HDRData& hdr)
        {
            for (int i = 0; i < size; ++i)
            {
                switch (sideData[i]->type)
                {
                case AV_FRAME_DATA_MASTERING_DISPLAY_METADATA:
                {
                    auto data = reinterpret_cast<AVMasteringDisplayMetadata*>(sideData[i]->data);
                    hdr.displayMasteringLuminance = dtk::RangeF(
                        data->min_luminance.num / data->min_luminance.den,
                        data->max_luminance.num / data->max_luminance.den);
                    break;
                }
                case AV_FRAME_DATA_CONTENT_LIGHT_LEVEL:
                {
                    auto data = reinterpret_cast<AVContentLightMetadata*>(sideData[i]->data);
                    hdr.maxCLL = data->MaxCLL;
                    hdr.maxFALL = data->MaxFALL;
                    break;
                }
                case AV_FRAME_DATA_DYNAMIC_HDR_PLUS:
                {
                    auto data = reinterpret_cast<AVDynamicHDRPlus*>(sideData[i]->data);
                    break;
                }
                default: break;
                }
            }
        }

        audio::DataType toAudioType(AVSampleFormat value)
        {
            audio::DataType out = audio::DataType::None;
            switch (value)
            {
            case AV_SAMPLE_FMT_S16:  out = audio::DataType::S16; break;
            case AV_SAMPLE_FMT_S32:  out = audio::DataType::S32; break;
            case AV_SAMPLE_FMT_FLT:  out = audio::DataType::F32; break;
            case AV_SAMPLE_FMT_DBL:  out = audio::DataType::F64; break;
            case AV_SAMPLE_FMT_S16P: out = audio::DataType::S16; break;
            case AV_SAMPLE_FMT_S32P: out = audio::DataType::S32; break;
            case AV_SAMPLE_FMT_FLTP: out = audio::DataType::F32; break;
            case AV_SAMPLE_FMT_DBLP: out = audio::DataType::F64; break;
            default: break;
            }
            return out;
        }

        AVSampleFormat fromAudioType(audio::DataType value)
        {
            AVSampleFormat out = AV_SAMPLE_FMT_NONE;
            switch (value)
            {
            case audio::DataType::S16: out = AV_SAMPLE_FMT_S16; break;
            case audio::DataType::S32: out = AV_SAMPLE_FMT_S32; break;
            case audio::DataType::F32: out = AV_SAMPLE_FMT_FLT; break;
            case audio::DataType::F64: out = AV_SAMPLE_FMT_DBL; break;
            default: break;
            }
            return out;
        }

        std::string getTimecodeFromDataStream(AVFormatContext* avFormatContext)
        {
            int dataStream = -1;
            for (unsigned int i = 0; i < avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_DATA == avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT == avFormatContext->streams[i]->disposition)
                {
                    dataStream = i;
                    break;
                }
            }
            if (-1 == dataStream)
            {
                for (unsigned int i = 0; i < avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_DATA == avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        dataStream = i;
                        break;
                    }
                }
            }
            std::string timecode;
            if (dataStream != -1)
            {
                AVDictionaryEntry* tag = nullptr;
                while ((tag = av_dict_get(
                    avFormatContext->streams[dataStream]->metadata,
                    "",
                    tag,
                    AV_DICT_IGNORE_SUFFIX)))
                {
                    if (dtk::compare(
                        tag->key,
                        "timecode",
                        dtk::CaseCompare::Insensitive))
                    {
                        timecode = tag->value;
                        break;
                    }
                }
            }
            return timecode;
        }

        Packet::Packet()
        {
            p = av_packet_alloc();
        }

        Packet::~Packet()
        {
            av_packet_free(&p);
        }

        std::string getErrorLabel(int r)
        {
            char buf[dtk::cStringSize];
            av_strerror(r, buf, dtk::cStringSize);
            return std::string(buf);
        }

        std::weak_ptr<dtk::LogSystem> ReadPlugin::_logSystemWeak;

        struct ReadPlugin::Private
        {
            std::vector<AVCodecID> codecIds;
            std::vector<std::string> codecNames;
        };

        void ReadPlugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            DTK_P();

            const AVCodec* avCodec = nullptr;
            void* avCodecIterate = nullptr;
            while ((avCodec = av_codec_iterate(&avCodecIterate)))
            {
                if ((AVMEDIA_TYPE_VIDEO == avCodec->type || AVMEDIA_TYPE_AUDIO == avCodec->type) &&
                    av_codec_is_decoder(avCodec))
                {
                    p.codecIds.push_back(avCodec->id);
                    p.codecNames.push_back(avCodec->name);
                }
            }

            std::map<std::string, io::FileType> extensions;
            const AVInputFormat* avInputFormat = nullptr;
            void* avInputFormatIterate = nullptr;
            std::vector<std::string> formatLog;
            while ((avInputFormat = av_demuxer_iterate(&avInputFormatIterate)))
            {
                if (avInputFormat->extensions)
                {
                    for (auto extension : dtk::split(avInputFormat->extensions, ','))
                    {
                        if (!extension.empty() && extension[0] != '.')
                        {
                            extension.insert(0, ".");
                        }
                        extensions[extension] = io::FileType::Movie;
                    }
                    formatLog.push_back(dtk::Format("    {0}: {1}").arg(avInputFormat->name).arg(avInputFormat->extensions));
                }
            }

            IReadPlugin::_init("FFmpeg", extensions, cache, logSystem);

            _logSystemWeak = logSystem;
            //av_log_set_level(AV_LOG_QUIET);
            av_log_set_level(AV_LOG_VERBOSE);
            av_log_set_callback(_logCallback);

            logSystem->print(
                "tl::io::ffmpeg::ReadPlugin",
                "Codecs: " + dtk::join(p.codecNames, ", "));
            logSystem->print(
                "tl::io::ffmpeg::ReadPlugin",
                "Formats:\n" + dtk::join(formatLog, '\n'));
        }

        ReadPlugin::ReadPlugin() :
            _p(new Private)
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(cache, logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, options, _cache, _logSystem.lock());
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _cache, _logSystem.lock());
        }

        void ReadPlugin::_logCallback(void*, int level, const char* fmt, va_list vl)
        {
            switch (level)
            {
            case AV_LOG_PANIC:
            case AV_LOG_FATAL:
            case AV_LOG_ERROR:
            case AV_LOG_WARNING:
            case AV_LOG_INFO:
                if (auto logSystem = _logSystemWeak.lock())
                {
                    char buf[dtk::cStringSize];
                    vsnprintf(buf, dtk::cStringSize, fmt, vl);
                    std::string s(buf);
                    dtk::removeTrailingNewlines(s);
                    logSystem->print("tl::io::ffmpeg::ReadPlugin", s);
                }
                break;
            case AV_LOG_VERBOSE:
            default: break;
            }
        }

        std::weak_ptr<dtk::LogSystem> WritePlugin::_logSystemWeak;

        struct WritePlugin::Private
        {
            std::vector<AVCodecID> codecIds;
            std::vector<std::string> codecNames;
        };

        void WritePlugin::_init(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            DTK_P();

            const AVCodec* avCodec = nullptr;
            void* avCodecIterate = nullptr;
            while ((avCodec = av_codec_iterate(&avCodecIterate)))
            {
                if (AVMEDIA_TYPE_VIDEO == avCodec->type && av_codec_is_encoder(avCodec))
                {
                    p.codecIds.push_back(avCodec->id);
                    p.codecNames.push_back(avCodec->name);
                }
            }

            std::map<std::string, io::FileType> extensions;
            const AVOutputFormat* avOutputFormat = nullptr;
            void* avOutputFormatIterate = nullptr;
            std::vector<std::string> formatLog;
            while ((avOutputFormat = av_muxer_iterate(&avOutputFormatIterate)))
            {
                if (avOutputFormat->extensions)
                {
                    bool match = false;
                    for (const auto id : p.codecIds)
                    {
                        if (av_codec_get_tag(avOutputFormat->codec_tag, id) != 0)
                        {
                            match = true;
                            break;
                        }
                    }
                    if (match)
                    {
                        for (auto extension : dtk::split(avOutputFormat->extensions, ','))
                        {
                            if (!extension.empty() && extension[0] != '.')
                            {
                                extension.insert(0, ".");
                            }                            
                            extensions[extension] = io::FileType::Movie;
                        }
                        formatLog.push_back(dtk::Format("    {0}: {1}").arg(avOutputFormat->name).arg(avOutputFormat->extensions));
                    }
                }
            }

            IWritePlugin::_init("FFmpeg", extensions, logSystem);

            //_logSystemWeak = logSystem;
            //av_log_set_level(AV_LOG_QUIET);
            //av_log_set_level(AV_LOG_VERBOSE);
            //av_log_set_callback(_logCallback);

            logSystem->print(
                "tl::io::ffmpeg::WritePlugin",
                "Codecs: " + dtk::join(p.codecNames, ", "));
            logSystem->print(
                "tl::io::ffmpeg::WritePlugin",
                "Formats:\n" + dtk::join(formatLog, '\n'));
        }

        WritePlugin::WritePlugin() :
            _p(new Private)
        {}

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(logSystem);
            return out;
        }

        const std::vector<std::string>& WritePlugin::getCodecs() const
        {
            return _p->codecNames;
        }

        dtk::ImageInfo WritePlugin::getInfo(
            const dtk::ImageInfo& info,
            const io::Options& options) const
        {
            dtk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case dtk::ImageType::L_U8:
            case dtk::ImageType::L_U16:
            case dtk::ImageType::RGB_U8:
            case dtk::ImageType::RGB_U16:
            case dtk::ImageType::RGBA_U8:
            case dtk::ImageType::RGBA_U16:
                out.type = info.type;
                break;
            default: break;
            }
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(dtk::Format("{0}: {1}").
                    arg(path.get()).
                    arg("Unsupported video"));
            return Write::create(path, info, options, _logSystem.lock());
        }

        void WritePlugin::_logCallback(void*, int level, const char* fmt, va_list vl)
        {
            switch (level)
            {
            case AV_LOG_PANIC:
            case AV_LOG_FATAL:
            case AV_LOG_ERROR:
            case AV_LOG_WARNING:
            case AV_LOG_INFO:
                if (auto logSystem = _logSystemWeak.lock())
                {
                    char buf[dtk::cStringSize];
                    vsnprintf(buf, dtk::cStringSize, fmt, vl);
                    std::string s(buf);
                    dtk::removeTrailingNewlines(s);
                    logSystem->print("tl::io::ffmpeg::WritePlugin", s);
                }
                break;
            case AV_LOG_VERBOSE:
            default: break;
            }
        }

        void to_json(nlohmann::json& json, const Options& value)
        {
            json["YUVToRGB"] = value.yuvToRgb;
            json["ThreadCount"] = value.threadCount;
        }

        void from_json(const nlohmann::json& json, Options& value)
        {
            json.at("YUVToRGB").get_to(value.yuvToRgb);
            json.at("ThreadCount").get_to(value.threadCount);
        }
    }
}
