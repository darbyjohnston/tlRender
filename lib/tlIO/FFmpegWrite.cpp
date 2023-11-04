// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlCore/StringFormat.h>
#include <tlCore/AudioResample.h>
#include <tlCore/LogSystem.h>

#include <tlIO/FFmpeg.h>

extern "C"
{
    
#include <libavutil/audio_fifo.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
    
#include <libavcodec/avcodec.h>
    
}

namespace tl
{
    namespace ffmpeg
    {
        namespace
        {
            
            AVSampleFormat toPlanarFormat(const enum AVSampleFormat s)
            {
                switch(s)
                {
                case AV_SAMPLE_FMT_U8:
                    return AV_SAMPLE_FMT_U8P;
                case AV_SAMPLE_FMT_S16:
                    return AV_SAMPLE_FMT_S16P;
                case AV_SAMPLE_FMT_S32:
                    return AV_SAMPLE_FMT_S32P;
                case AV_SAMPLE_FMT_FLT:
                    return AV_SAMPLE_FMT_FLTP;
                case AV_SAMPLE_FMT_DBL:
                    return AV_SAMPLE_FMT_DBLP;
                default:
                    return s;
                }
            }
            
            //! Check that a given sample format is supported by the encoder
            bool checkSampleFormat(const AVCodec* codec,
                                   enum AVSampleFormat sample_fmt)
            {
                const enum AVSampleFormat *p = codec->sample_fmts;

                while (*p != AV_SAMPLE_FMT_NONE) {
                    if (*p == sample_fmt)
                        return true;
                    p++;
                }
                return false;
            }

            //! Select layout with equal or the highest channel count
            int selectChannelLayout(const AVCodec* codec, AVChannelLayout* dst,
                                    int channelCount)
            {
                const AVChannelLayout *p, *best_ch_layout;
                int best_nb_channels   = 0;
                
                if (!codec->ch_layouts)
                {
                    if (channelCount == 2)
                    {
                        AVChannelLayout channel_layout = AV_CHANNEL_LAYOUT_STEREO;
                        return av_channel_layout_copy(dst, &channel_layout);
                    }
                    else if (channelCount == 1)
                    {
                        AVChannelLayout channel_layout = AV_CHANNEL_LAYOUT_MONO;
                        return av_channel_layout_copy(dst, &channel_layout);
                    }
                    av_channel_layout_default(dst, channelCount);
                    return 0;
                }

                p = codec->ch_layouts;
                while (p->nb_channels) {
                    int nb_channels = p->nb_channels;
                    
                    if (nb_channels > best_nb_channels) {
                        best_ch_layout   = p;
                        best_nb_channels = nb_channels;
                    }
                    p++;
                }
                return av_channel_layout_copy(dst, best_ch_layout);
            }
            
            //! Return an equal or higher supported samplerate
            int selectSampleRate(const AVCodec* codec, const int sampleRate)
            {
                const int *p;
                int best_samplerate = 0;

                if (!codec->supported_samplerates)
                    return 44100;

                p = codec->supported_samplerates;
                while (*p) {

                    if (*p == sampleRate)
                        return sampleRate;
                    
                    if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
                        best_samplerate = *p;
                    p++;
                }
                return best_samplerate;
            }
        }
        
        struct Write::Private
        {
            std::string fileName;
            AVFormatContext* avFormatContext = nullptr;

            // Video
            AVCodecContext* avCodecContext = nullptr;
            AVStream* avVideoStream = nullptr;
            AVPacket* avPacket = nullptr;
            AVFrame* avFrame = nullptr;
            AVPixelFormat avPixelFormatIn = AV_PIX_FMT_NONE;
            AVFrame* avFrame2 = nullptr;
            SwsContext* swsContext = nullptr;
            otime::RationalTime videoStartTime = time::invalidTime;

            // Audio
            AVCodecContext* avAudioCodecContext = nullptr;
            AVStream* avAudioStream = nullptr;
            AVAudioFifo* avAudioFifo = nullptr;
            AVFrame* avAudioFrame = nullptr;
            AVPacket* avAudioPacket = nullptr;
            bool avAudioPlanar = false;
            uint64_t totalSamples = 0;
            int64_t audioStartSamples = 0;
            size_t  sampleRate = 0;
            std::shared_ptr<audio::AudioResample> resample;
            std::vector<uint8_t*> flatData;
            
            bool opened = false;
        };

        void Write::_init(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);

            TLRENDER_P();

            p.fileName = path.get();
            if (info.video.empty() && !info.audio.isValid())
            {
                throw std::runtime_error(string::Format("{0}: No video or audio").arg(p.fileName));
            }

            int r = avformat_alloc_output_context2(
                &p.avFormatContext, NULL, NULL, p.fileName.c_str());
            if (r < 0)
                throw std::runtime_error(
                    string::Format(
                        "{0}: Could not allocate output context")
                    .arg(p.fileName));
            
            AVCodec* avCodec = nullptr;
            AVCodecID avCodecID = AV_CODEC_ID_AAC;
            auto option = options.find("FFmpeg/AudioCodec");
            if (option != options.end())
            {
                AudioCodec audioCodec;
                std::stringstream ss(option->second);
                ss >> audioCodec;
                switch (audioCodec)
                {
                case AudioCodec::None:
                    avCodecID = AV_CODEC_ID_NONE;
                    break;
                case AudioCodec::AAC:
                    avCodecID = AV_CODEC_ID_AAC;
                    break;
                case AudioCodec::AC3:
                    avCodecID = AV_CODEC_ID_AC3;
                    break;
                case AudioCodec::MP3:
                    avCodecID = AV_CODEC_ID_MP3;
                    break;
                case AudioCodec::PCM_S16LE:
                    avCodecID = AV_CODEC_ID_PCM_S16LE;
                    break;
                default:
                {
                    const std::string codec = ss.str();
                    const char* name = codec.c_str();
                    avCodec = const_cast<AVCodec*>(avcodec_find_encoder_by_name(name));
                    if (!avCodec)
                    {
                        const AVCodecDescriptor* desc =
                            avcodec_descriptor_get_by_name(name);
                        if (desc)
                        {
                            avCodecID = desc->id;
                        }
                    }
                    break;
                }
                }

                // Sanity check on codecs and containers.
                const std::string extension =
                    string::toLower(path.getExtension());
                if (extension == ".wav")
                {
                    if (avCodecID != AV_CODEC_ID_PCM_S16LE &&
                        avCodecID != AV_CODEC_ID_MP3 &&
                        avCodecID != AV_CODEC_ID_AAC)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                "Invalid codec for .wav, switching to AAC",
                                log::Type::Error);
                        }
                        avCodecID = AV_CODEC_ID_AAC;
                    }
                }
                else if (extension == ".aiff")
                {
                    if (avCodecID != AV_CODEC_ID_PCM_S16LE)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                "Invalid codec for .aiff, switching to PCM_S16LE",
                                log::Type::Error);
                        }
                        avCodecID = AV_CODEC_ID_PCM_S16LE;
                    }
                }
                else if (extension == ".mp3")
                {
                    if (avCodecID != AV_CODEC_ID_MP3)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::io::ffmpeg::Plugin::Write",
                                "Invalid codec for .mp3, switching to MP3 (needs libmp3lame)",
                                log::Type::Error);
                        }
                        avCodecID = AV_CODEC_ID_MP3;
                    }
                }
            }
                
            if (info.audio.isValid() && avCodecID != AV_CODEC_ID_NONE)
            {
                if (!avCodec)
                    avCodec = const_cast<AVCodec*>(avcodec_find_encoder(avCodecID));
                if (!avCodec)
                    throw std::runtime_error("Could not find audio encoder");
                
                p.avAudioStream = avformat_new_stream(p.avFormatContext,
                                                      avCodec);
                if (!p.avAudioStream)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate audio stream")
                            .arg(p.fileName));
                }

                p.avAudioStream->id = p.avFormatContext->nb_streams - 1;
                
                p.avAudioCodecContext = avcodec_alloc_context3(avCodec);
                if (!p.avAudioCodecContext)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Cannot allocate audio codec context")
                            .arg(p.fileName));
                }

                bool resample = false;
                p.avAudioCodecContext->sample_fmt =
                    fromAudioType(info.audio.dataType);
                if (!checkSampleFormat(
                        avCodec, p.avAudioCodecContext->sample_fmt))
                {
                    // Try it as a planar format then.
                    AVSampleFormat planarFormat = 
                        toPlanarFormat(p.avAudioCodecContext->sample_fmt);

                    if (!checkSampleFormat(avCodec, planarFormat))
                    {
                        // If that also failed, initialize a resampler
                        resample = true;

                        if (checkSampleFormat(avCodec, AV_SAMPLE_FMT_FLT))
                        {
                            p.avAudioPlanar = false;
                            p.avAudioCodecContext->sample_fmt =
                                AV_SAMPLE_FMT_FLT;
                        }
                        else if(checkSampleFormat(avCodec, AV_SAMPLE_FMT_FLTP))
                        {
                            p.avAudioPlanar = true;
                            p.avAudioCodecContext->sample_fmt =
                                AV_SAMPLE_FMT_FLTP;
                        }
                        else if(checkSampleFormat(avCodec, AV_SAMPLE_FMT_S16))
                        {
                            p.avAudioPlanar = false;
                            p.avAudioCodecContext->sample_fmt =
                                AV_SAMPLE_FMT_S16;
                        }
                        else
                        {
                            throw std::runtime_error(
                                string::Format(
                                    "Sample format {0} not supported!")
                                    .arg(av_get_sample_fmt_name(
                                        p.avAudioCodecContext->sample_fmt)));
                        }
                    }
                    else
                    {
                        p.avAudioCodecContext->sample_fmt = planarFormat;
                        p.avAudioPlanar = true;
                    }
                }

                if (p.avAudioPlanar)
                    p.flatData.resize(channels);
                else
                    p.flatData.resize(1);
                
                r = selectChannelLayout(
                    avCodec, &p.avAudioCodecContext->ch_layout,
                    info.audio.channelCount);
                if (r < 0)
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Could not select audio channel layout")
                            .arg(p.fileName));

                p.sampleRate =
                    selectSampleRate(avCodec, info.audio.sampleRate);
                if (p.sampleRate == 0)
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Could not select sample rate")
                            .arg(p.fileName));
                    
                char buf[256];
                av_channel_layout_describe(
                    &p.avAudioCodecContext->ch_layout, buf, 256);
                    
                if (p.sampleRate != info.audio.sampleRate || resample)
                {
                    const audio::Info& input = info.audio;
                    audio::Info output(info.audio.channelCount,
                                       toAudioType(p.avAudioCodecContext->sample_fmt),
                                       p.sampleRate);
                    p.resample = audio::AudioResample::create(input, output);
                    
                    if (auto logSystem = _logSystem.lock())
                    {
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin::Write",
                            string::Format(
                                "Resample from layout {0}, {1} channels, type "
                                "{2}, sample rate {3} to layout {4}, {5} "
                                "channels, type {6}, sample rate {7}.")
                            .arg(buf)
                            .arg(input.channelCount)
                            .arg(input.dataType)
                            .arg(input.sampleRate)
                            .arg(buf)
                            .arg(output.channelCount)
                            .arg(output.dataType)
                            .arg(output.sampleRate));
                    }
                }
                else
                {
                    const audio::Info& input = info.audio;
                    if (auto logSystem = _logSystem.lock())
                    {
                        logSystem->print(
                            "tl::io::ffmpeg::Plugin::Write",
                            string::Format(
                                "Save from layout {0}, {1} channels, type "
                                "{2}, sample rate {3}.")
                            .arg(buf)
                            .arg(input.channelCount)
                            .arg(input.dataType)
                            .arg(input.sampleRate));
                    }
                }

                p.avAudioCodecContext->bit_rate = 69000;
                p.avAudioCodecContext->sample_rate = p.sampleRate;
                p.avAudioCodecContext->time_base.num = 1;
                p.avAudioCodecContext->time_base.den = p.sampleRate;

                if ((p.avAudioCodecContext->block_align == 1 ||
                     p.avAudioCodecContext->block_align == 1152 ||
                     p.avAudioCodecContext->block_align == 576) &&
                    p.avAudioCodecContext->codec_id == AV_CODEC_ID_MP3)
                    p.avAudioCodecContext->block_align = 0;

                if (avCodecID == AV_CODEC_ID_AC3)
                    p.avAudioCodecContext->block_align = 0;

                r = avcodec_open2(p.avAudioCodecContext, avCodec, NULL);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Could not open audio codec - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                r = avcodec_parameters_from_context(
                    p.avAudioStream->codecpar, p.avAudioCodecContext);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Could not copy parameters from context - {1}.")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }
    
                p.avAudioPacket = av_packet_alloc();
                if (!p.avAudioPacket)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate audio packet")
                            .arg(p.fileName));
                }

                p.avAudioFifo = av_audio_fifo_alloc(
                    p.avAudioCodecContext->sample_fmt, info.audio.channelCount,
                    1); // cannot be 0, must be 1 at least
                if (!p.avAudioFifo)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: Cannot allocate audio FIFO buffer - {1}.")
                        .arg(p.fileName)
                        .arg(getErrorLabel(r)));
                }
                    
                p.avAudioFrame = av_frame_alloc();
                if (!p.avAudioFrame)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot allocate audio frame - {1}.")
                        .arg(p.fileName)
                        .arg(getErrorLabel(r)));
                }

                if (p.avAudioCodecContext->codec->capabilities &
                    AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
                {
                    p.avAudioCodecContext->frame_size = p.sampleRate;
                }
                p.avAudioFrame->nb_samples = p.avAudioCodecContext->frame_size;
                p.avAudioFrame->format = p.avAudioCodecContext->sample_fmt;
                p.avAudioFrame->sample_rate = p.sampleRate;
                r = av_channel_layout_copy(
                    &p.avAudioFrame->ch_layout,
                    &p.avAudioCodecContext->ch_layout);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Could not copy channel layout to "
                                       "audio frame - {1}.")
                        .arg(p.fileName)
                        .arg(getErrorLabel(r)));
                }

                r = av_frame_get_buffer(p.avAudioFrame, 0);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Could not allocate buffer for "
                                       "audio frame - {1}.")
                        .arg(p.fileName)
                        .arg(getErrorLabel(r)));
                }
            }

            if (!info.video.empty())
            {
                AVCodecID avCodecID = AV_CODEC_ID_MPEG4;
                Profile profile = Profile::None;
                int avProfile = FF_PROFILE_UNKNOWN;
                auto option = options.find("FFmpeg/WriteProfile");
                if (option != options.end())
                {
                    std::stringstream ss(option->second);
                    ss >> profile;
                }
                switch (profile)
                {
                case Profile::H264:
                    avCodecID = AV_CODEC_ID_H264;
                    avProfile = FF_PROFILE_H264_HIGH;
                    break;
                case Profile::ProRes:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = FF_PROFILE_PRORES_STANDARD;
                    break;
                case Profile::ProRes_Proxy:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = FF_PROFILE_PRORES_PROXY;
                    break;
                case Profile::ProRes_LT:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = FF_PROFILE_PRORES_LT;
                    break;
                case Profile::ProRes_HQ:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = FF_PROFILE_PRORES_HQ;
                    break;
                case Profile::ProRes_4444:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = FF_PROFILE_PRORES_4444;
                    break;
                case Profile::ProRes_XQ:
                    avCodecID = AV_CODEC_ID_PRORES;
                    avProfile = FF_PROFILE_PRORES_XQ;
                    break;
                default: break;
                }
                
                const AVCodec* avCodec = avcodec_find_encoder(avCodecID);
                if (!avCodec)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot find encoder").arg(p.fileName));
                }
                p.avCodecContext = avcodec_alloc_context3(avCodec);
                if (!p.avCodecContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(p.fileName));
                }
                p.avVideoStream = avformat_new_stream(p.avFormatContext, avCodec);
                if (!p.avVideoStream)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate stream").arg(p.fileName));
                }
                p.avVideoStream->id = p.avFormatContext->nb_streams - 1;
                if (!avCodec->pix_fmts)
                {
                    throw std::runtime_error(string::Format("{0}: No pixel formats available").arg(p.fileName));
                }

                p.avCodecContext->codec_id = avCodec->id;
                p.avCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
                const auto& videoInfo = info.video[0];
                p.avCodecContext->width = videoInfo.size.w;
                p.avCodecContext->height = videoInfo.size.h;
                p.avCodecContext->sample_aspect_ratio = AVRational({ 1, 1 });
                p.avCodecContext->pix_fmt = avCodec->pix_fmts[0];
                const auto rational = time::toRational(info.videoTime.duration().rate());
                p.avCodecContext->time_base = { rational.second, rational.first };
                p.avCodecContext->framerate = { rational.first, rational.second };
                p.avCodecContext->profile = avProfile;
                if (p.avFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
                {
                    p.avCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                }
                p.avCodecContext->thread_count = 0;
                p.avCodecContext->thread_type = FF_THREAD_FRAME;

                r = avcodec_open2(p.avCodecContext, avCodec, NULL);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: avcodec_open2 - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                r = avcodec_parameters_from_context(p.avVideoStream->codecpar, p.avCodecContext);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "{0}: avcodec_parameters_from_context - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                p.avVideoStream->time_base = { rational.second, rational.first };
                p.avVideoStream->avg_frame_rate = { rational.first, rational.second };

                for (const auto& i : info.tags)
                {
                    av_dict_set(&p.avFormatContext->metadata, i.first.c_str(), i.second.c_str(), 0);
                }

                // Set timecode
                option = options.find("timecode");
                if (option != options.end())
                {
                    std::stringstream ss(option->second);
                    std::string timecode;
                    ss >> timecode;
                    r = av_dict_set(
                        &p.avFormatContext->metadata, "timecode", timecode.c_str(), 0);
                    if (r < 0)
                        throw std::runtime_error(string::Format("Could not set timecode to {1}")
                                                 .arg(timecode));
                
                    otime::ErrorStatus errorStatus;
                    const otime::RationalTime time = otime::RationalTime::from_timecode(
                        timecode,
                        info.videoTime.duration().rate(),
                        &errorStatus);
                    if (!otime::is_error(errorStatus))
                    {
                        p.videoStartTime = time::floor(time);
                    }
                }

                p.avPacket = av_packet_alloc();
                if (!p.avPacket)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate packet").arg(p.fileName));
                }

                p.avFrame = av_frame_alloc();
                if (!p.avFrame)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(p.fileName));
                }
                p.avFrame->format = p.avVideoStream->codecpar->format;
                p.avFrame->width = p.avVideoStream->codecpar->width;
                p.avFrame->height = p.avVideoStream->codecpar->height;
                r = av_frame_get_buffer(p.avFrame, 0);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: av_frame_get_buffer - {1}")
                            .arg(p.fileName)
                            .arg(getErrorLabel(r)));
                }

                p.avFrame2 = av_frame_alloc();
                if (!p.avFrame2)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(p.fileName));
                }
                switch (videoInfo.pixelType)
                {
                case image::PixelType::L_U8:     p.avPixelFormatIn = AV_PIX_FMT_GRAY8;  break;
                case image::PixelType::RGB_U8:   p.avPixelFormatIn = AV_PIX_FMT_RGB24;  break;
                case image::PixelType::RGBA_U8:  p.avPixelFormatIn = AV_PIX_FMT_RGBA;   break;
                case image::PixelType::L_U16:    p.avPixelFormatIn = AV_PIX_FMT_GRAY16; break;
                case image::PixelType::RGB_U16:  p.avPixelFormatIn = AV_PIX_FMT_RGB48;  break;
                case image::PixelType::RGBA_U16: p.avPixelFormatIn = AV_PIX_FMT_RGBA64; break;
                default:
                    throw std::runtime_error(string::Format("{0}: Incompatible pixel type").arg(p.fileName));
                    break;
                }
                /*p.swsContext = sws_getContext(
                  videoInfo.size.w,
                  videoInfo.size.h,
                  p.avPixelFormatIn,
                  videoInfo.size.w,
                  videoInfo.size.h,
                  p.avCodecContext->pix_fmt,
                  swsScaleFlags,
                  0,
                  0,
                  0);*/
                p.swsContext = sws_alloc_context();
                if (!p.swsContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(p.fileName));
                }
                av_opt_set_defaults(p.swsContext);
                r = av_opt_set_int(p.swsContext, "srcw", videoInfo.size.w, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(p.swsContext, "srch", videoInfo.size.h, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(p.swsContext, "src_format", p.avPixelFormatIn, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(p.swsContext, "dstw", videoInfo.size.w, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(p.swsContext, "dsth", videoInfo.size.h, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(p.swsContext, "dst_format", p.avCodecContext->pix_fmt, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(p.swsContext, "sws_flags", swsScaleFlags, AV_OPT_SEARCH_CHILDREN);
                r = av_opt_set_int(p.swsContext, "threads", 0, AV_OPT_SEARCH_CHILDREN);
                r = sws_init_context(p.swsContext, nullptr, nullptr);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot initialize sws context").arg(p.fileName));
                }
            }
            
            if (p.avFormatContext->nb_streams == 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: No video or audio streams.")
                        .arg(p.fileName));
            }
                
            //av_dump_format(p.avFormatContext, 0, p.fileName.c_str(), 1);

            r = avio_open(&p.avFormatContext->pb, p.fileName.c_str(), AVIO_FLAG_WRITE);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: avio_open - {1}")
                                             .arg(p.fileName)
                                             .arg(getErrorLabel(r)));
            }
    
            r = avformat_write_header(p.avFormatContext, NULL);
            if (r < 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: avformat_write_header - {1}")
                        .arg(p.fileName)
                        .arg(getErrorLabel(r)));
            }
            
            p.opened = true;
        }

        Write::Write() :
            _p(new Private)
        {}

        Write::~Write()
        {
            TLRENDER_P();
            
            if (p.opened)
            {
                // We need to enclose this in a try block as _encode can throw
                try
                {
                    if (p.avAudioCodecContext)
                    {
                        _flushAudio();
                    
                        _encode(
                            p.avAudioCodecContext, p.avAudioStream, nullptr,
                            p.avAudioPacket);
                    }

                    if (p.avCodecContext)
                    {
                        _encode(
                            p.avCodecContext, p.avVideoStream, nullptr,
                            p.avPacket);
                    }
                }
                catch(const std::exception&)
                {
                }
                av_write_trailer(p.avFormatContext);
            }

            if (p.swsContext)
            {
                sws_freeContext(p.swsContext);
            }
            if (p.avFrame2)
            {
                av_frame_free(&p.avFrame2);
            }
            if (p.avFrame)
            {
                av_frame_free(&p.avFrame);
            }
            if (p.avAudioFrame)
            {
                av_frame_free(&p.avAudioFrame);
            }
            if (p.avPacket)
            {
                av_packet_free(&p.avPacket);
            }
            if (p.avAudioPacket)
            {
                av_packet_free(&p.avAudioPacket);
            }
            if (p.avAudioFifo)
            {
                av_audio_fifo_free( p.avAudioFifo );
                p.avAudioFifo = nullptr;
            }
            if (p.avAudioCodecContext)
            {
                avcodec_free_context(&p.avAudioCodecContext);
            }
            if (p.avCodecContext)
            {
                avcodec_free_context(&p.avCodecContext);
            }
            if (p.avFormatContext && p.avFormatContext->pb)
            {
                avio_closep(&p.avFormatContext->pb);
            }
            if (p.avFormatContext)
            {
                avformat_free_context(p.avFormatContext);
            }
        }

        std::shared_ptr<Write> Write::create(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::writeVideo(
            const otime::RationalTime& time,
            const std::shared_ptr<image::Image>& image,
            const io::Options&)
        {
            TLRENDER_P();
            if (!p.avCodecContext)
                return;
            
            const auto& info = image->getInfo();
            av_image_fill_arrays(
                p.avFrame2->data,
                p.avFrame2->linesize,
                image->getData(),
                p.avPixelFormatIn,
                info.size.w,
                info.size.h,
                info.layout.alignment);

            // Flip the image vertically.
            switch (info.pixelType)
            {
            case image::PixelType::L_U8:
            case image::PixelType::L_U16:
            case image::PixelType::RGB_U8:
            case image::PixelType::RGB_U16:
            case image::PixelType::RGBA_U8:
            case image::PixelType::RGBA_U16:
            {
                const size_t channelCount = image::getChannelCount(info.pixelType);
                for (size_t i = 0; i < channelCount; i++)
                {
                    p.avFrame2->data[i] += p.avFrame2->linesize[i] * (info.size.h - 1);
                    p.avFrame2->linesize[i] = -p.avFrame2->linesize[i];
                }
                break;
            }
            case image::PixelType::YUV_420P_U8:
            case image::PixelType::YUV_422P_U8:
            case image::PixelType::YUV_444P_U8:
            case image::PixelType::YUV_420P_U16:
            case image::PixelType::YUV_422P_U16:
            case image::PixelType::YUV_444P_U16:
                //! \bug How do we flip YUV data?
                throw std::runtime_error(string::Format("{0}: Incompatible pixel type").arg(p.fileName));
                break;
            default: break;
            }

            sws_scale(
                p.swsContext,
                (uint8_t const* const*)p.avFrame2->data,
                p.avFrame2->linesize,
                0,
                p.avVideoStream->codecpar->height,
                p.avFrame->data,
                p.avFrame->linesize);

            const auto timeRational = time::toRational(time.rate());
            p.avFrame->pts = av_rescale_q(
                time.value() - p.videoStartTime.value(),
                { timeRational.second, timeRational.first },
                p.avVideoStream->time_base);

            _encode(
                    p.avCodecContext, p.avVideoStream, p.avFrame,
                    p.avPacket);
        }

        void Write::writeAudio(
            const otime::TimeRange& inTimeRange,
            const std::shared_ptr<audio::Audio>& audioIn,
            const io::Options&)
        {
            TLRENDER_P();
            
            if (!audioIn || !p.avAudioFifo || audioIn->getSampleCount() == 0)
                return;
            
            const auto& info = audioIn->getInfo();
            if (!info.isValid())
                return;
            
            int r = 0;
            const auto timeRange = otime::TimeRange(
                inTimeRange.start_time().rescaled_to(p.sampleRate),
                inTimeRange.duration().rescaled_to(p.sampleRate));

            int fifoSize = av_audio_fifo_size(p.avAudioFifo);
                
            if (timeRange.start_time().value() >=
                p.totalSamples + p.audioStartSamples + fifoSize)
            {
                // If this is the start of the saving, store the start time.
                if (p.totalSamples == 0)
                {
                    p.audioStartSamples = timeRange.start_time().value();
                }

                auto audioResampled = audioIn;
                // Resample audio
                if (p.resample)
                {
                    audioResampled = p.resample->process(audioIn);
                }
                
                // Most codecs need non-interleaved audio.
                std::shared_ptr<audio::Audio> audio;
                if (p.avAudioPlanar)
                    audio = planarDeinterleave(audioResampled);
                else
                    audio = audioResampled;
                
                uint8_t* data = audio->getData();
                
                // Allocate flatData pointers
                if (p.avAudioPlanar)
                {
                    const size_t channels = audio->getChannelCount();
                    const size_t stride = audio->getByteCount() / channels;
                    for (size_t i = 0; i < channels; ++i)
                    {
                        p.flatData[i] = data + i * stride;
                    }
                }
                else
                {
                    p.flatData[0] = data;
                }
                
                const size_t sampleCount = audio->getSampleCount();
                r = av_audio_fifo_write(
                    p.avAudioFifo, reinterpret_cast<void**>(p.flatData.data()),
                    sampleCount);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "Could not write to fifo buffer at {0}.")
                        .arg(timeRange));
                }
                if (r != sampleCount)
                {
                    throw std::runtime_error(
                        string::Format(
                            "Could not write all samples fifo buffer at {0}.")
                        .arg(timeRange));
                }
            }

            const AVRational ratio = { 1, p.avAudioCodecContext->sample_rate };

            const int frameSize = p.avAudioCodecContext->frame_size;
            while (av_audio_fifo_size(p.avAudioFifo) >= frameSize)
            {
                r = av_frame_make_writable(p.avAudioFrame);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format(
                            "Could not make audio frame writable at time {0}.")
                        .arg(timeRange));
                }
                
                r = av_audio_fifo_read(
                    p.avAudioFifo,
                    reinterpret_cast<void**>(p.avAudioFrame->extended_data),
                    frameSize);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("Could not read from audio fifo buffer "
                                       "at {0}.")
                            .arg(timeRange));
                }

                p.avAudioFrame->pts = av_rescale_q(
                    p.totalSamples, ratio,
                    p.avAudioCodecContext->time_base);

                _encode(
                    p.avAudioCodecContext, p.avAudioStream, p.avAudioFrame,
                    p.avAudioPacket);
                
                p.totalSamples += frameSize;
            }
        }
        
        void Write::_flushAudio()
        {
            TLRENDER_P();

            // If FIFO still has some data, send it
            const AVRational ratio = { 1, p.avAudioCodecContext->sample_rate };
            int fifoSize = av_audio_fifo_size(p.avAudioFifo);
            if (fifoSize > 0)
            {
                int r = av_frame_make_writable(p.avAudioFrame);
                if (r < 0)
                    return;

                p.avAudioFrame->nb_samples = fifoSize;

                r = av_audio_fifo_read(
                    p.avAudioFifo,
                    reinterpret_cast<void**>(p.avAudioFrame->extended_data),
                    fifoSize);
                if (r < 0)
                    return;

                p.avAudioFrame->pts = av_rescale_q(
                    p.totalSamples, ratio,
                    p.avAudioCodecContext->time_base);

                _encode(
                    p.avAudioCodecContext, p.avAudioStream, p.avAudioFrame,
                    p.avAudioPacket);
            }
        }
        
        void Write::_encode(AVCodecContext* context,
                            const AVStream* stream,
                            const AVFrame* frame,
                            AVPacket* packet)
        {
            TLRENDER_P();
                
            int r = avcodec_send_frame(context, frame);
            if (r < 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot send frame")
                        .arg(p.fileName));
            }

            while (r >= 0)
            {
                r = avcodec_receive_packet(context, packet);
                if (r == AVERROR(EAGAIN) || r == AVERROR_EOF)
                {
                    return;
                }
                else if (r < 0) 
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot receive packet")
                            .arg(p.fileName));
                }


                packet->stream_index = stream->index; // Needed

                r = av_interleaved_write_frame(p.avFormatContext, packet);
                if (r < 0)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot write frame")
                        .arg(p.fileName));
                }
                av_packet_unref(packet);
            }
        
        }


    }
}
