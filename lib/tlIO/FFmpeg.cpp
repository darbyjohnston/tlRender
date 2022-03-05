// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpeg.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>

} // extern "C"

#include <array>

namespace tl
{
    namespace ffmpeg
    {
        TLRENDER_ENUM_IMPL(
            Profile,
            "H264",
            "ProRes",
            "ProRes_Proxy",
            "ProRes_LT",
            "ProRes_HQ",
            "ProRes_4444",
            "ProRes_XQ");
        TLRENDER_ENUM_SERIALIZE_IMPL(Profile);

        AVRational swap(AVRational value)
        {
            return AVRational({ value.den, value.num });
        }

        int64_t fromChannelCount(uint8_t value)
        {
            int64_t out = 0;
            switch (value)
            {
            case 1: out = AV_CH_LAYOUT_MONO; break;
            case 2: out = AV_CH_LAYOUT_STEREO; break;
            case 6: out = AV_CH_LAYOUT_5POINT1; break;
            case 7: out = AV_CH_LAYOUT_6POINT1; break;
            case 8: out = AV_CH_LAYOUT_7POINT1; break;
            }
            return out;
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

        std::string getErrorLabel(int r)
        {
            char buf[string::cBufferSize];
            av_strerror(r, buf, string::cBufferSize);
            return std::string(buf);
        }

        std::weak_ptr<log::System> Plugin::_logSystemWeak;

        void Plugin::_init(const std::weak_ptr<log::System>& logSystem)
        {
            IPlugin::_init(
                "FFmpeg",
                {
                    { ".mov", io::FileType::Movie },
                    { ".m4v", io::FileType::Movie },
                    { ".mp4", io::FileType::Movie },
                    { ".y4m", io::FileType::Movie },
                    { ".mkv", io::FileType::Movie },
                    { ".mxf", io::FileType::Movie },
                    { ".wmv", io::FileType::Movie },
                    { ".wav", io::FileType::Audio },
                    { ".mp3", io::FileType::Audio },
                    { ".aiff", io::FileType::Audio }
                },
                logSystem);

            _logSystemWeak = logSystem;
            //av_log_set_level(AV_LOG_QUIET);
            av_log_set_level(AV_LOG_VERBOSE);
            av_log_set_callback(_logCallback);

            av_register_all();
            avcodec_register_all();
            /*AVCodec* avCodec = nullptr;
            std::vector<std::string> codecNames;
            while ((avCodec = av_codec_next(avCodec)))
            {
                codecNames.push_back(avCodec->name);
            }
            logSystem->print("tl::io::ffmpeg::Plugin", "Codecs: " + string::join(codecNames, ", "));*/
        }

        Plugin::Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, io::merge(options, _options), _logSystem);
        }

        imaging::Info Plugin::getWriteInfo(
            const imaging::Info& info,
            const io::Options& options) const
        {
            imaging::Info out;
            out.size = info.size;
            switch (info.pixelType)
            {
            case imaging::PixelType::L_U8:
            case imaging::PixelType::RGB_U8:
            case imaging::PixelType::RGBA_U8:
            case imaging::PixelType::YUV_420P:
                out.pixelType = info.pixelType;
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
                throw std::runtime_error(string::Format("{0}: {1}").
                    arg(path.get()).
                    arg("Unsupported video"));
            return Write::create(path, info, io::merge(options, _options), _logSystem);
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
            case AV_LOG_VERBOSE:
                if (auto logSystem = _logSystemWeak.lock())
                {
                    char buf[string::cBufferSize];
                    vsnprintf(buf, string::cBufferSize, fmt, vl);
                    logSystem->print("tl::io::ffmpeg::Plugin", string::removeTrailingNewlines(buf));
                }
                break;
            default: break;
            }
        }
    }
}
