// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FFmpeg.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Error.h>
#include <tlrCore/LogSystem.h>
#include <tlrCore/String.h>
#include <tlrCore/StringFormat.h>

extern "C"
{
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>

} // extern "C"

#include <array>

namespace tlr
{
    namespace ffmpeg
    {
        TLR_ENUM_IMPL(
            Profile,
            "H264",
            "ProRes",
            "ProRes_Proxy",
            "ProRes_LT",
            "ProRes_HQ",
            "ProRes_4444",
            "ProRes_XQ");
        TLR_ENUM_SERIALIZE_IMPL(Profile);

        AVRational swap(AVRational value)
        {
            return AVRational({ value.den, value.num });
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

        std::string getErrorLabel(int r)
        {
            char buf[string::cBufferSize];
            av_strerror(r, buf, string::cBufferSize);
            return std::string(buf);
        }

        std::weak_ptr<core::LogSystem> Plugin::_logSystemWeak;

        void Plugin::_init(const std::shared_ptr<core::LogSystem>& logSystem)
        {
            IPlugin::_init(
                "FFmpeg",
                { ".mov", ".m4v", ".mp4", ".y4m", ".mkv", ".mxf", ".wav", ".mp3", ".aiff" },
                logSystem);

            _logSystemWeak = logSystem;
            //av_log_set_level(AV_LOG_QUIET);
            av_log_set_level(AV_LOG_VERBOSE);
            av_log_set_callback(_logCallback);
            
            av_register_all();
            avcodec_register_all();
            AVCodec* avCodec = nullptr;
            std::vector<std::string> codecNames;
            while ((avCodec = av_codec_next(avCodec)))
            {
                codecNames.push_back(avCodec->name);
            }
            logSystem->print("tlr::ffmpeg::Plugin", "Codecs: " + string::join(codecNames, ", "));
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create(const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const file::Path& path,
            const avio::Options& options)
        {
            return Read::create(path, avio::merge(options, _options), _logSystem);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::L_U8,
                imaging::PixelType::RGB_U8,
                imaging::PixelType::RGBA_U8,
                imaging::PixelType::YUV_420P
            };
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            return !info.video.empty() && _isWriteCompatible(info.video[0]) ?
                Write::create(path, info, avio::merge(options, _options), _logSystem) :
                nullptr;
        }

        void Plugin::_logCallback(void*, int level, const char* fmt, va_list vl)
        {
            switch (level)
            {
            /*case AV_LOG_PANIC:
            case AV_LOG_FATAL:
            case AV_LOG_ERROR:
            case AV_LOG_WARNING:
            case AV_LOG_INFO:
            case AV_LOG_VERBOSE:
                if (auto logSystem = _logSystemWeak.lock())
                {
                    char buf[string::cBufferSize];
                    vsnprintf(buf, string::cBufferSize, fmt, vl);
                    logSystem->print("tlr::ffmpeg::Plugin", string::removeTrailingNewlines(buf));
                }
                break;*/
            default: break;
            }
        }
    }
}
