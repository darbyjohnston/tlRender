// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpeg.h>

#include <dtk/core/Assert.h>
#include <dtk/core/Error.h>
#include <dtk/core/Format.h>
#include <dtk/core/LogSystem.h>
#include <dtk/core/String.h>

extern "C"
{
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/hdr_dynamic_metadata.h>
#include <libavutil/imgutils.h>
#include <libavutil/mastering_display_metadata.h>
}

#include <array>

namespace tl
{
    namespace ffmpeg
    {
        DTK_ENUM_IMPL(
            Profile,
            "None",
            "H264",
            "ProRes",
            "ProRes_Proxy",
            "ProRes_LT",
            "ProRes_HQ",
            "ProRes_4444",
            "ProRes_XQ");

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

        std::weak_ptr<dtk::LogSystem> Plugin::_logSystemWeak;

        void Plugin::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            IPlugin::_init(
                "FFmpeg",
                {
                    { ".mov", io::FileType::Movie },
                    { ".m4v", io::FileType::Movie },
                    { ".mp4", io::FileType::Movie },
                    { ".y4m", io::FileType::Movie },
                    { ".mxf", io::FileType::Movie },
                    { ".wmv", io::FileType::Movie },
                    { ".avi", io::FileType::Movie },
                    { ".wav", io::FileType::Audio },
                    { ".mp3", io::FileType::Audio },
                    { ".aiff", io::FileType::Audio }
                },
                cache,
                logSystem);

            _logSystemWeak = logSystem;
            //av_log_set_level(AV_LOG_QUIET);
            av_log_set_level(AV_LOG_VERBOSE);
            av_log_set_callback(_logCallback);

            const AVCodec* avCodec = nullptr;
            void* avCodecIterate = nullptr;
            std::vector<std::string> codecNames;
            while ((avCodec = av_codec_iterate(&avCodecIterate)))
            {
                codecNames.push_back(avCodec->name);
            }
            //std::cout << dtk::join(codecNames, ", ") << std::endl;
            if (auto logSystem = _logSystemWeak.lock())
            {
                logSystem->print("tl::io::ffmpeg::Plugin", "Codecs: " + dtk::join(codecNames, ", "));
            }
        }

        Plugin::Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(cache, logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, options, _cache, _logSystem.lock());
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _cache, _logSystem.lock());
        }

        dtk::ImageInfo Plugin::getWriteInfo(
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

        std::shared_ptr<io::IWrite> Plugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isWriteCompatible(info.video[0], options)))
                throw std::runtime_error(dtk::Format("{0}: {1}").
                    arg(path.get()).
                    arg("Unsupported video"));
            return Write::create(path, info, options, _logSystem.lock());
        }

        void Plugin::_logCallback(void*, int level, const char* fmt, va_list vl)
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
                    logSystem->print("tl::io::ffmpeg::Plugin", s);
                }
                break;
            case AV_LOG_VERBOSE:
            default: break;
            }
        }
    }
}
