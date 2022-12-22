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
                p.audio.avCodecContext[p.audio.avStream]->thread_count = p.options.threadCount;
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
                if (p.options.audioConvertInfo.isValid())
                {
                    channelCount = p.options.audioConvertInfo.channelCount;
                    dataType = p.options.audioConvertInfo.dataType;
                    sampleRate = p.options.audioConvertInfo.sampleRate;
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

        void Read::Private::startAudio(const std::string& fileName)
        {
            if (audio.avStream != -1)
            {
                audio.avFrame = av_frame_alloc();
                if (!audio.avFrame)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot allocate frame").arg(fileName));
                }

                AVChannelLayout channelLayout;
                av_channel_layout_default(&channelLayout, info.audio.channelCount);
                const auto& avCodecParameters = audio.avCodecParameters[audio.avStream];
                int r = swr_alloc_set_opts2(
                    &audio.swrContext,
                    &channelLayout,
                    fromAudioType(info.audio.dataType),
                    info.audio.sampleRate,
                    &avCodecParameters->ch_layout,
                    static_cast<AVSampleFormat>(avCodecParameters->format),
                    avCodecParameters->sample_rate,
                    0,
                    NULL);
                av_channel_layout_uninit(&channelLayout);
                if (!audio.swrContext)
                {
                    throw std::runtime_error(string::Format("{0}: Cannot get context").arg(fileName));
                }
                swr_init(audio.swrContext);
            }
        }

        void Read::Private::processAudio()
        {
            bool seek = false;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if (currentAudioRequest)
                {
                    if (!time::compareExact(currentAudioRequest->time.start_time(), currentAudioTime))
                    {
                        seek = true;
                        currentAudioTime = currentAudioRequest->time.start_time();
                    }
                }
            }
            if (seek)
            {
                //std::cout << "audio seek: " << currentAudioTime << std::endl;

                if (audio.avStream != -1)
                {
                    avcodec_flush_buffers(audio.avCodecContext[audio.avStream]);

                    AVRational r;
                    r.num = 1;
                    r.den = info.audio.sampleRate;
                    if (av_seek_frame(
                        audio.avFormatContext,
                        audio.avStream,
                        av_rescale_q(
                            currentAudioTime.value() - info.audioTime.start_time().value(),
                            r,
                            audio.avFormatContext->streams[audio.avStream]->time_base),
                        AVSEEK_FLAG_BACKWARD) < 0)
                    {
                        //! \todo How should this be handled?
                    }

                    std::vector<uint8_t> swrOutputBuffer;
                    swrOutputBuffer.resize(
                        static_cast<size_t>(info.audio.channelCount) *
                        audio::getByteCount(info.audio.dataType) *
                        audio.avFrame->nb_samples);
                    uint8_t* swrOutputBufferP[] = { swrOutputBuffer.data() };
                    while (swr_convert(
                        audio.swrContext,
                        swrOutputBufferP,
                        audio.avFrame->nb_samples,
                        NULL,
                        0) > 0)
                        ;
                    swr_init(audio.swrContext);
                }

                audio.buffer.clear();
            }

            if (audio.avStream != -1 &&
                audio::getSampleCount(audio.buffer) <
                options.audioBufferSize.rescaled_to(info.audio.sampleRate).value())
            {
                AVPacket packet;
                av_init_packet(&packet);
                int decoding = 0;
                bool eof = false;
                while (0 == decoding)
                {
                    if (!eof)
                    {
                        decoding = av_read_frame(audio.avFormatContext, &packet);
                        if (AVERROR_EOF == decoding)
                        {
                            eof = true;
                            decoding = 0;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                    }
                    if ((eof && audio.avStream != -1) || (audio.avStream == packet.stream_index))
                    {
                        decoding = avcodec_send_packet(
                            audio.avCodecContext[audio.avStream],
                            eof ? nullptr : &packet);
                        if (AVERROR_EOF == decoding)
                        {
                            decoding = 0;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                        decoding = decodeAudio();
                        if (AVERROR(EAGAIN) == decoding)
                        {
                            decoding = 0;
                        }
                        else if (AVERROR_EOF == decoding)
                        {
                            const size_t bufferSize = audio::getSampleCount(audio.buffer);
                            const size_t bufferMax = options.audioBufferSize.rescaled_to(info.audio.sampleRate).value();
                            if (bufferSize < bufferMax)
                            {
                                auto audio = audio::Audio::create(
                                    info.audio,
                                    bufferMax - bufferSize);
                                audio->zero();
                                this->audio.buffer.push_back(audio);
                            }
                            break;
                        }
                        else if (decoding < 0)
                        {
                            //! \todo How should this be handled?
                            break;
                        }
                        else if (1 == decoding)
                        {
                            break;
                        }
                    }
                    if (packet.buf)
                    {
                        av_packet_unref(&packet);
                    }
                }
                if (packet.buf)
                {
                    av_packet_unref(&packet);
                }
                //std::cout << "audio buffer size: " << audio::getSampleCount(audio.buffer) << std::endl;
            }

            const size_t bufferSize = audio::getSampleCount(audio.buffer);
            std::shared_ptr<Private::AudioRequest> request;
            {
                std::unique_lock<std::mutex> lock(mutex);
                if ((currentAudioRequest &&
                    currentAudioRequest->time.duration().rescaled_to(info.audio.sampleRate).value() <= bufferSize) ||
                    (currentAudioRequest && -1 == audio.avStream))
                {
                    request = std::move(currentAudioRequest);
                }
            }
            if (request)
            {
                io::AudioData data;
                data.time = request->time.start_time();
                data.audio = audio::Audio::create(info.audio, request->time.duration().value());
                data.audio->zero();
                size_t offset = 0;
                if (data.time < info.audioTime.start_time())
                {
                    offset = (info.audioTime.start_time() - data.time).value() * info.audio.getByteCount();
                }
                audio::copy(audio.buffer, data.audio->getData() + offset, data.audio->getByteCount() - offset);
                request->promise.set_value(data);

                currentAudioTime += request->time.duration();
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
