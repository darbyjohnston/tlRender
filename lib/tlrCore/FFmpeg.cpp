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

        void extractAudio(
            uint8_t**                     in,
            int                           format,
            uint8_t                       channelCount,
            std::shared_ptr<audio::Audio> out)
        {
            const uint8_t outChannelCount = out->getChannelCount();
            switch (format)
            {
            case AV_SAMPLE_FMT_S16:
            {
                if (channelCount == outChannelCount)
                {
                    memcpy(out->getData(), in[0], out->getByteCount());
                }
                else
                {
                    audio::extract(
                        reinterpret_cast<int16_t*>(in[0]),
                        reinterpret_cast<int16_t*>(out->getData()),
                        out->getSampleCount(),
                        channelCount,
                        outChannelCount);
                }
                break;
            }
            case AV_SAMPLE_FMT_S32:
            {
                if (channelCount == outChannelCount)
                {
                    memcpy(out->getData(), in[0], out->getByteCount());
                }
                else
                {
                    audio::extract(
                        reinterpret_cast<int32_t*>(in[0]),
                        reinterpret_cast<int32_t*>(out->getData()),
                        out->getSampleCount(),
                        channelCount,
                        outChannelCount);
                }
                break;
            }
            case AV_SAMPLE_FMT_FLT:
            {
                if (channelCount == outChannelCount)
                {
                    memcpy(out->getData(), in[0], out->getByteCount());
                }
                else
                {
                    audio::extract(
                        reinterpret_cast<float*>(in[0]),
                        reinterpret_cast<float*>(out->getData()),
                        out->getSampleCount(),
                        channelCount,
                        outChannelCount);
                }
                break;
            }
            case AV_SAMPLE_FMT_DBL:
            {
                if (channelCount == outChannelCount)
                {
                    memcpy(out->getData(), in[0], out->getByteCount());
                }
                else
                {
                    audio::extract(
                        reinterpret_cast<double*>(in[0]),
                        reinterpret_cast<double*>(out->getData()),
                        out->getSampleCount(),
                        channelCount,
                        outChannelCount);
                }
                break;
            }
            case AV_SAMPLE_FMT_S16P:
                audio::planarInterleave(
                    const_cast<const int16_t**>(reinterpret_cast<int16_t**>(in)),
                    reinterpret_cast<int16_t*>(out->getData()),
                    outChannelCount,
                    out->getSampleCount());
                break;
            case AV_SAMPLE_FMT_S32P:
                audio::planarInterleave(
                    const_cast<const int32_t**>(reinterpret_cast<int32_t**>(in)),
                    reinterpret_cast<int32_t*>(out->getData()),
                    outChannelCount,
                    out->getSampleCount());
                break;
            case AV_SAMPLE_FMT_FLTP:
                audio::planarInterleave(
                    const_cast<const float**>(reinterpret_cast<float**>(in)),
                    reinterpret_cast<float*>(out->getData()),
                    outChannelCount,
                    out->getSampleCount());
                break;
            case AV_SAMPLE_FMT_DBLP:
                audio::planarInterleave(
                    const_cast<const double**>(reinterpret_cast<double**>(in)),
                    reinterpret_cast<double*>(out->getData()),
                    outChannelCount,
                    out->getSampleCount());
                break;
            default: break;
            }
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
                { ".mov", ".m4v", ".mp4", ".y4m", ".mkv", ".mxf" },
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
