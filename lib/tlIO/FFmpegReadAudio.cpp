// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace ffmpeg
    {
        void Read::_openAudio(const std::string& fileName)
        {
            TLRENDER_P();

            if (!_memory.empty())
            {
                p.audio.avFormatContext = avformat_alloc_context();
                if (!p.audio.avFormatContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate format context").arg(fileName));
                }

                p.audio.avIOBufferData = AVIOBufferData(_memory[0].p, _memory[0].size);
                p.audio.avIOContextBuffer = static_cast<uint8_t*>(av_malloc(avIOContextBufferSize));
                p.audio.avIOContext = avio_alloc_context(
                    p.audio.avIOContextBuffer,
                    avIOContextBufferSize,
                    0,
                    &p.audio.avIOBufferData,
                    &avIOBufferRead,
                    nullptr,
                    &avIOBufferSeek);
                if (!p.audio.avIOContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate I/O context").arg(fileName));
                }

                p.audio.avFormatContext->pb = p.audio.avIOContext;
            }

            int r = avformat_open_input(
                &p.audio.avFormatContext,
                !p.audio.avFormatContext ? fileName.c_str() : nullptr,
                nullptr,
                nullptr);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }

            r = avformat_find_stream_info(p.audio.avFormatContext, 0);
            if (r < 0)
            {
                throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
            }
            for (unsigned int i = 0; i < p.audio.avFormatContext->nb_streams; ++i)
            {
                if (AVMEDIA_TYPE_AUDIO == p.audio.avFormatContext->streams[i]->codecpar->codec_type &&
                    AV_DISPOSITION_DEFAULT == p.audio.avFormatContext->streams[i]->disposition)
                {
                    p.audio.avStream = i;
                    break;
                }
            }
            if (-1 == p.audio.avStream)
            {
                for (unsigned int i = 0; i < p.audio.avFormatContext->nb_streams; ++i)
                {
                    if (AVMEDIA_TYPE_AUDIO == p.audio.avFormatContext->streams[i]->codecpar->codec_type)
                    {
                        p.audio.avStream = i;
                        break;
                    }
                }
            }
            if (p.audio.avStream != -1)
            {
                //av_dump_format(p.audio.avFormatContext, p.audio.avStream, fileName.c_str(), 0);

                auto avAudioStream = p.audio.avFormatContext->streams[p.audio.avStream];
                auto avAudioCodecParameters = avAudioStream->codecpar;
                auto avAudioCodec = avcodec_find_decoder(avAudioCodecParameters->codec_id);
                if (!avAudioCodec)
                {
                    throw std::runtime_error(string::Format("{0}: No audio codec found").arg(fileName));
                }
                p.audio.avCodecParameters[p.audio.avStream] = avcodec_parameters_alloc();
                if (!p.audio.avCodecParameters[p.audio.avStream])
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate parameters").arg(fileName));
                }
                r = avcodec_parameters_copy(p.audio.avCodecParameters[p.audio.avStream], avAudioCodecParameters);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.audio.avCodecContext[p.audio.avStream] = avcodec_alloc_context3(avAudioCodec);
                if (!p.audio.avCodecContext[p.audio.avStream])
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate context").arg(fileName));
                }
                r = avcodec_parameters_to_context(p.audio.avCodecContext[p.audio.avStream], p.audio.avCodecParameters[p.audio.avStream]);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }
                p.audio.avCodecContext[p.audio.avStream]->thread_count = p.threadCount;
                p.audio.avCodecContext[p.audio.avStream]->thread_type = FF_THREAD_FRAME;
                r = avcodec_open2(p.audio.avCodecContext[p.audio.avStream], avAudioCodec, 0);
                if (r < 0)
                {
                    throw std::runtime_error(string::Format("{0}: {1}").arg(fileName).arg(getErrorLabel(r)));
                }

                const size_t fileChannelCount = p.audio.avCodecParameters[p.audio.avStream]->ch_layout.nb_channels;
                switch (fileChannelCount)
                {
                case 1:
                case 2:
                case 6:
                case 7:
                case 8: break;
                default:
                    throw std::runtime_error(string::Format("{0}: Unsupported audio channels").arg(fileName));
                    break;
                }
                const audio::DataType fileDataType = toAudioType(static_cast<AVSampleFormat>(
                    p.audio.avCodecParameters[p.audio.avStream]->format));
                if (audio::DataType::None == fileDataType)
                {
                    throw std::runtime_error(string::Format("{0}: Unsupported audio format").arg(fileName));
                }
                const size_t fileSampleRate = p.audio.avCodecParameters[p.audio.avStream]->sample_rate;

                size_t channelCount = fileChannelCount;
                audio::DataType dataType = fileDataType;
                size_t sampleRate = fileSampleRate;
                if (p.audioConvertInfo.isValid())
                {
                    channelCount = p.audioConvertInfo.channelCount;
                    dataType = p.audioConvertInfo.dataType;
                    sampleRate = p.audioConvertInfo.sampleRate;
                }

                int64_t sampleCount = 0;
                if (avAudioStream->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = 1;
                    r.den = sampleRate;
                    sampleCount = av_rescale_q(
                        avAudioStream->duration,
                        avAudioStream->time_base,
                        r);
                }
                else if (p.audio.avFormatContext->duration != AV_NOPTS_VALUE)
                {
                    AVRational r;
                    r.num = 1;
                    r.den = sampleRate;
                    sampleCount = av_rescale_q(
                        p.audio.avFormatContext->duration,
                        av_get_time_base_q(),
                        r);
                }

                imaging::Tags tags;
                AVDictionaryEntry* tag = nullptr;
                otime::RationalTime startTime(0.0, sampleRate);
                while ((tag = av_dict_get(p.audio.avFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                {
                    const std::string key(tag->key);
                    const std::string value(tag->value);
                    tags[key] = value;
                    if (string::compareNoCase(key, "timecode"))
                    {
                        otime::ErrorStatus errorStatus;
                        const otime::RationalTime time = otime::RationalTime::from_timecode(
                            value,
                            p.currentVideoTime.rate(),
                            &errorStatus);
                        if (!otime::is_error(errorStatus))
                        {
                            startTime = time::floor(time.rescaled_to(sampleRate));
                            //std::cout << "start time: " << startTime << std::endl;
                        }
                    }
                }

                p.info.audio.channelCount = channelCount;
                p.info.audio.dataType = dataType;
                p.info.audio.sampleRate = sampleRate;
                p.info.audioTime = otime::TimeRange(
                    startTime,
                    otime::RationalTime(sampleCount, sampleRate));

                p.currentAudioTime = p.info.audioTime.start_time();

                for (const auto& i : tags)
                {
                    p.info.tags[i.first] = i.second;
                }
                {
                    std::stringstream ss;
                    ss << static_cast<int>(fileChannelCount);
                    p.info.tags["Audio Channels"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << fileDataType;
                    p.info.tags["Audio Data Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(1);
                    ss << std::fixed;
                    ss << fileSampleRate / 1000.F << " kHz";
                    p.info.tags["Audio Sample Rate"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << p.info.audioTime.start_time().rescaled_to(1.0).value() << " seconds";
                    p.info.tags["Audio Start Time"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << p.info.audioTime.duration().rescaled_to(1.0).value() << " seconds";
                    p.info.tags["Audio Duration"] = ss.str();
                }
            }
        }

        int Read::Private::decodeAudio()
        {
            int out = 0;
            while (0 == out)
            {
                out = avcodec_receive_frame(audio.avCodecContext[audio.avStream], audio.avFrame);
                if (out < 0)
                {
                    return out;
                }
                const int64_t timestamp = audio.avFrame->pts != AV_NOPTS_VALUE ? audio.avFrame->pts : audio.avFrame->pkt_dts;
                //std::cout << "audio timestamp: " << timestamp << std::endl;

                AVRational r;
                r.num = 1;
                r.den = info.audio.sampleRate;
                const auto time = otime::RationalTime(
                    info.audioTime.start_time().value() +
                    av_rescale_q(
                        timestamp,
                        audio.avFormatContext->streams[audio.avStream]->time_base,
                        r),
                    info.audio.sampleRate);
                //std::cout << "audio time: " << time << std::endl;

                if (time >= currentAudioTime)
                {
                    //std::cout << "audio time: " << time << std::endl;
                    const int64_t swrDelay = swr_get_delay(audio.swrContext, audio.avCodecParameters[audio.avStream]->sample_rate);
                    //std::cout << "delay: " << swrDelay << std::endl;
                    const size_t swrOutputSamples = audio.avFrame->nb_samples + swrDelay;
                    std::vector<uint8_t> swrOutputBuffer;
                    swrOutputBuffer.resize(
                        static_cast<size_t>(info.audio.channelCount) *
                        audio::getByteCount(info.audio.dataType) *
                        swrOutputSamples);
                    uint8_t* swrOutputBufferP[] = { swrOutputBuffer.data() };
                    const int swrOutputCount = swr_convert(
                        audio.swrContext,
                        swrOutputBufferP,
                        swrOutputSamples,
                        (const uint8_t **)audio.avFrame->data,
                        audio.avFrame->nb_samples);
                    if (swrOutputCount < 0)
                    {
                        //! \todo How should this be handled?
                        break;
                    }
                    auto tmp = audio::Audio::create(info.audio, swrOutputCount);
                    memcpy(tmp->getData(), swrOutputBuffer.data(), tmp->getByteCount());
                    audio.buffer.push_back(tmp);
                    out = 1;
                    break;
                }
            }
            return out;
        }
    }
}
