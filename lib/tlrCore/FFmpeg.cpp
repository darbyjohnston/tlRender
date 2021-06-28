// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FFmpeg.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Error.h>
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
            VideoCodec,
            "H264",
            "H265",
            "DNxHD",
            "ProRes");

        AVRational toRational(double value)
        {
            const std::array<AVRational, 6> common =
            {
                AVRational({ 24, 1 }),
                AVRational({ 30, 1 }),
                AVRational({ 60, 1 }),
                AVRational({ 24000, 1001 }),
                AVRational({ 30000, 1001 }),
                AVRational({ 60000, 1001 })
            };
            const double tolerance = 0.01;
            for (const auto& i : common)
            {
                const double diff = abs(value - i.num / static_cast<double>(i.den));
                if (diff < tolerance)
                {
                    return i;
                }
            }
            return AVRational({ static_cast<int>(value), 1 });
        }

        AVRational swap(AVRational value)
        {
            return AVRational({ value.den, value.num });
        }

        std::string getErrorLabel(int r)
        {
            char buf[string::cBufferSize];
            av_strerror(r, buf, string::cBufferSize);
            return std::string(buf);
        }

        void Plugin::_init()
        {
            IPlugin::_init(
                "FFmpeg",
                { ".mov", ".m4v", ".mp4", ".y4m", ".mkv" });

            av_log_set_level(AV_LOG_QUIET);
            //av_log_set_level(AV_LOG_VERBOSE);
            
            av_register_all();
            avcodec_register_all();
            //AVCodec* avCodec = nullptr;
            //while (avCodec = av_codec_next(avCodec))
            //{
            //    std::cout << "codec: " << avCodec->name << std::endl;
            //}
        }

        Plugin::Plugin()
        {}
            
        std::shared_ptr<Plugin> Plugin::create()
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init();
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const std::string& fileName,
            const avio::Options& options)
        {
            return Read::create(fileName, options);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::L_U8,
                imaging::PixelType::RGB_U8,
                imaging::PixelType::RGBA_U8,
            };
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const std::string& fileName,
            const avio::Info& info,
            const avio::Options& options)
        {
            return !info.video.empty() && _isWriteCompatible(info.video[0]) ?
                Write::create(fileName, info, options) :
                nullptr;
        }
    }

    TLR_ENUM_SERIALIZE_IMPL(ffmpeg, VideoCodec);
}
