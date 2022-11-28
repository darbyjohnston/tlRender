// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/AudioConvert.h>

#if defined(TLRENDER_FFMPEG)
extern "C"
{
#include <libswresample/swresample.h>
}
#endif // TLRENDER_FFMPEG

namespace tl
{
    namespace audio
    {
        namespace
        {
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
        }

        struct AudioConvert::Private
        {
            audio::Info inputInfo;
            audio::Info outputInfo;
#if defined(TLRENDER_FFMPEG)
            SwrContext* swrContext = nullptr;
#endif // TLRENDER_FFMPEG
        };

        void AudioConvert::_init(
            const audio::Info& inputInfo,
            const audio::Info& outputInfo)
        {
            TLRENDER_P();
            p.inputInfo = inputInfo;
            p.outputInfo = outputInfo;
#if defined(TLRENDER_FFMPEG)
            p.swrContext = swr_alloc_set_opts(
                NULL,
                fromChannelCount(p.outputInfo.channelCount),
                fromAudioType(p.outputInfo.dataType),
                p.outputInfo.sampleRate,
                fromChannelCount(p.inputInfo.channelCount),
                fromAudioType(p.inputInfo.dataType),
                p.inputInfo.sampleRate,
                0,
                NULL);
            if (p.swrContext)
            {
                swr_init(p.swrContext);
            }
#endif // TLRENDER_FFMPEG
        }

        AudioConvert::AudioConvert() :
            _p(new Private())
        {}

        AudioConvert::~AudioConvert()
        {
            TLRENDER_P();
#if defined(TLRENDER_FFMPEG)
            if (p.swrContext)
            {
                swr_free(&p.swrContext);
            }
#endif // TLRENDER_FFMPEG
        }

        std::shared_ptr<AudioConvert> AudioConvert::create(
            const audio::Info& inputInfo,
            const audio::Info& outputInfo)
        {
            auto out = std::shared_ptr<AudioConvert>(new AudioConvert);
            out->_init(inputInfo, outputInfo);
            return out;
        }

        const audio::Info& AudioConvert::getInputInfo() const
        {
            return _p->inputInfo;
        }

        const audio::Info& AudioConvert::getOutputInfo() const
        {
            return _p->outputInfo;
        }

        std::vector<uint8_t> AudioConvert::convert(const uint8_t* value, size_t sampleCount)
        {
            TLRENDER_P();
            std::vector<uint8_t> out;
#if defined(TLRENDER_FFMPEG)
            if (p.swrContext && value)
            {
                const int64_t swrDelay = swr_get_delay(p.swrContext, p.inputInfo.sampleRate);
                //std::cout << "delay: " << swrDelay << std::endl;
                const size_t swrOutputSamples = sampleCount + swrDelay;
                out.resize(swrOutputSamples * p.outputInfo.getByteCount());
                uint8_t* swrOutputBufferP[] = { out.data() };
                const uint8_t* swrInputBufferP[] = { value };
                const int swrOutputCount = swr_convert(
                    p.swrContext,
                    swrOutputBufferP,
                    swrOutputSamples,
                    swrInputBufferP,
                    sampleCount);
            }
#endif // TLRENDER_FFMPEG
            return out;
        }

        void AudioConvert::flush()
        {
            TLRENDER_P();
#if defined(TLRENDER_FFMPEG)
            while (swr_convert(
                p.swrContext,
                NULL,
                0,
                NULL,
                0) > 0)
                ;
#endif // TLRENDER_FFMPEG
        }
    }
}
